// Copyright 2020 The TensorFlow Runtime Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//===- bef_executor_driver.cc - Library for bef_executor test driver ------===//
//
// This file implements the test driver library for the bef executor. It opens
// up a given mlir file and then runs it with a host executor.
//
//===----------------------------------------------------------------------===//
#include "tfrt/bef_executor_driver/bef_executor_driver.h"

#include <limits>

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/SourceMgr.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "tfrt/bef_executor/bef_file.h"
#include "tfrt/core_runtime/core_runtime.h"
#include "tfrt/core_runtime/tensor_handle.h"
#include "tfrt/host_context/async_value.h"
#include "tfrt/host_context/concurrent_work_queue.h"
#include "tfrt/host_context/function.h"
#include "tfrt/host_context/host_allocator.h"
#include "tfrt/host_context/host_context.h"
#include "tfrt/host_context/kernel_registry.h"
#include "tfrt/host_context/location.h"
#include "tfrt/metrics/metrics_api.h"
#include "tfrt/support/mutex.h"
#include "tfrt/support/ostream.h"
#include "tfrt/support/string_util.h"
#include "tfrt/tracing/tracing.h"

namespace tfrt {

int RunBefExecutor(const RunBefConfig& run_config) {
  TFRT_TRACE_SCOPE("Bef Executor");
  static auto* version_metric =
      metrics::NewGauge<std::string>("/tensorflow/runtime/version");
  static std::once_flag initialized;
  std::call_once(initialized, [] { version_metric->SetValue("TFRT_V0"); });

  // Set up the input file.
  std::string error_message;
  auto file = mlir::openInputFile(run_config.input_filename, &error_message);
  if (!file) {
    llvm::errs() << error_message << "\n";
    return 1;
  }

  // Tell source_mgr about this buffer, which is what the parser will pick up.
  llvm::SourceMgr source_mgr;
  source_mgr.AddNewSourceBuffer(std::move(file), llvm::SMLoc());

  // Parse the input file.
  mlir::MLIRContext context;
  mlir::SourceMgrDiagnosticVerifierHandler source_mgr_handler(source_mgr,
                                                              &context);

  auto decoded_diagnostic_handler = [&](const DecodedDiagnostic& diag) {
    std::string message = "runtime error: " + diag.message;

    auto decoded_loc = diag.location;
    if (decoded_loc) {
      auto loc =
          mlir::FileLineColLoc::get(decoded_loc->filename, decoded_loc->line,
                                    decoded_loc->column, &context);
      emitError(loc) << message;
    } else {
      auto loc = mlir::FileLineColLoc::get("", 0, 0, &context);
      emitError(loc) << message;
    }
  };

  assert(GetNumReferenceCountedObjects() == 0 &&
         "We have reference-counted objects before we started to do anything");

  std::unique_ptr<HostAllocator> host_allocator;
  switch (run_config.host_allocator_type) {
    case HostAllocatorType::kMalloc:
      host_allocator = CreateMallocAllocator();
      tfrt::outs() << "Choosing malloc.\n";
      break;
    case HostAllocatorType::kTestFixedSizeMalloc:
      host_allocator = tfrt::CreateFixedSizeAllocator();
      tfrt::outs() << "Choosing fixed size malloc.\n";
      break;
    case HostAllocatorType::kProfiledMalloc:
      host_allocator = CreateMallocAllocator();
      host_allocator = CreateProfiledAllocator(std::move(host_allocator));
      tfrt::outs() << "Choosing profiled allocator based on malloc.\n";
      break;
    case HostAllocatorType::kLeakCheckMalloc:
      host_allocator = CreateMallocAllocator();
      host_allocator = CreateLeakCheckAllocator(std::move(host_allocator));
      tfrt::outs() << "Choosing memory leak check allocator.\n";
  }
  tfrt::outs().flush();

  // Dig the bytes out of the SourceMgr.
  auto buffer =
      source_mgr.getMemoryBuffer(source_mgr.getMainFileID())->getBuffer();
  auto buffer_arr = llvm::ArrayRef<uint8_t>(
      reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());

  std::unique_ptr<ConcurrentWorkQueue> work_queue =
      CreateWorkQueue(run_config.work_queue_type);
  if (work_queue == nullptr) {
    llvm::errs() << run_config.program_name
                 << ": couldn't create work queue type "
                 << run_config.work_queue_type << "\n";
    return 1;
  }
  tfrt::outs() << "Choosing " << work_queue->name() << " work queue.\n";
  tfrt::outs().flush();

  assert(AsyncValue::GetNumAsyncValueInstances() == 0 &&
         "We have async values allocated before we started to do anything");
  auto async_value_guard = llvm::make_scope_exit([]() {
    assert(AsyncValue::GetNumAsyncValueInstances() == 0 &&
           "All async values should be cleaned up at the end");
    assert(GetNumReferenceCountedObjects() == 0 &&
           "We have live reference-counted objects before exit.");
  });

  auto core_rt =
      CoreRuntime::Create(decoded_diagnostic_handler, std::move(host_allocator),
                          std::move(work_queue), run_config.devices);
  if (!core_rt) {
    llvm::errs() << core_rt.takeError();
    return 1;
  }

  auto* host = core_rt.get()->GetHostContext();

  // If there are any libraries specified, load them and see if they have a
  // kernel registration function.
  for (const auto& lib_name : run_config.shared_libs) {
    std::string err;
    auto dyn_lib =
        llvm::sys::DynamicLibrary::getPermanentLibrary(lib_name.c_str(), &err);
    if (!dyn_lib.isValid()) {
      llvm::errs() << run_config.program_name << ": couldn't load library "
                   << err << "\n";
      return 1;
    }

    // The library should specify a kernel registration entrypoint.
    if (auto kernel_reg = dyn_lib.SearchForAddressOfSymbol("RegisterKernels")) {
      reinterpret_cast<void (*)(KernelRegistry*)>(kernel_reg)(
          host->GetRegistry());
    }
  }

  auto bef(BEFFile::Open(buffer_arr, host->GetRegistry(),
                         decoded_diagnostic_handler, host->allocator()));

  if (!bef) {
    return mlir::failed(source_mgr_handler.verify());
  }

  SmallVector<const Function*, 8> function_list;

  if (run_config.functions.empty()) {
    // No functions specified in the command line. Try to run all functions in
    // the input BEF file.
    bef->GetFunctionList(&function_list);
  } else {
    function_list.reserve(run_config.functions.size());

    for (auto& fn_name : run_config.functions) {
      auto* fn = bef->GetFunction(fn_name);

      if (!fn) {
        llvm::errs() << run_config.program_name << ": couldn't find function "
                     << fn_name << "\n";
        return 1;
      }
      function_list.push_back(fn);
    }
  }

  // Loop over each of the functions, running each as a standalone testcase.
  for (auto* fn : function_list) {
    TFRT_TRACE_KERNEL_SCOPE(StrCat("Function: ", fn->name()));
    // If the function takes arguments, then we can't run it from this driver.
    if (!fn->argument_types().empty()) {
      tfrt::outs() << "--- Not running '" << fn->name()
                   << "' because it has arguments.\n";
      tfrt::outs().flush();
      continue;
    }

    // Skip anonymous functions.
    if (fn->name().empty()) {
      continue;
    }

    size_t before_num_values;
    if (AsyncValue::AsyncValueAllocationTrackingEnabled())
      before_num_values = AsyncValue::GetNumAsyncValueInstances();

    tfrt::outs() << "--- Running '" << fn->name() << "':\n";
    tfrt::outs().flush();

    // Kick off an execution of the function body.
    llvm::SmallVector<RCReference<AsyncValue>, 4> results;
    results.resize(fn->result_types().size());
    fn->Execute(/*arguments=*/{}, results, host);

    // Block until the function results are fully resolved.
    host->Await(results);

    // Go ahead and print out the function results that we know about.
    if (!results.empty()) {
      tfrt::outs() << "'" << fn->name() << "' returned ";
      auto result_types = fn->result_types();

      for (int i = 0, e = results.size(); i != e; ++i) {
        auto type_name = result_types[i];
        if (auto* error = results[i]->GetErrorIfPresent()) {
          tfrt::outs() << "<<error: " << error->message << ">>";
        } else if (type_name.GetName() == "i1") {
          tfrt::outs() << results[i]->get<bool>();
        } else if (type_name.GetName() == "i32") {
          tfrt::outs() << results[i]->get<int32_t>();
        } else if (type_name.GetName() == "i64") {
          tfrt::outs() << results[i]->get<int64_t>();
        } else if (type_name.GetName() == "f32") {
          tfrt::outs() << results[i]->get<float>();
        } else if (type_name.GetName() == "f64") {
          tfrt::outs() << results[i]->get<double>();
        } else {
          tfrt::outs() << type_name.GetName() << " value";
        }

        // Print comma except for the last one.
        if (i != results.size() - 1) {
          tfrt::outs() << ',';
        }
      }

      tfrt::outs() << '\n';
      tfrt::outs().flush();
    }

    // In this test driver, we want to make sure that every function completes
    // all execution before moving on to the next one.  This makes the leak
    // checker work better in the face of side effecting kernels that aren't
    // properly chained together (which is useful for testing).
    host->Quiesce();

    // Always call Restart() to clear the cancel async value. The execution of
    // a BEF function may cause HostContext to enter the canceled state.
    host->Restart();

    // Drop any result references before doing the leak check.
    results.clear();

    if (AsyncValue::AsyncValueAllocationTrackingEnabled()) {
      auto after_num_values = AsyncValue::GetNumAsyncValueInstances();
      if (before_num_values != after_num_values) {
        llvm::errs() << "Evaluation of function '" << fn->name() << "' leaked "
                     << (after_num_values - before_num_values)
                     << " async values (before: " << before_num_values
                     << ", after: " << after_num_values << ")!\n";
        abort();
      }
    }
  }

  bef.reset();
  // Verify the diagnostic handler to make sure that each of the diagnostics
  // matched.
  return mlir::failed(source_mgr_handler.verify());
}

}  // namespace tfrt
