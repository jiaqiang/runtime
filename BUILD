load(":build_defs.bzl", "tfrt_cc_library")
load("@tf_runtime//third_party/mlir:tblgen.bzl", "gentbl")

package(
    default_visibility = [":__subpackages__"],
)

licenses(["notice"])

package_group(
    name = "friends",
    packages = [
        "//...",
    ],
)

exports_files([
    "LICENSE",
])

# To build tf_runtime without RTTI/exceptions, use:
# bazel build --define=disable_rtti_and_exceptions=true
config_setting(
    name = "disable_rtti_and_exceptions",
    define_values = {"disable_rtti_and_exceptions": "true"},
    visibility = ["//visibility:public"],
)

tfrt_cc_library(
    name = "hostcontext",
    srcs = [
        "lib/host_context/async_value.cc",
        "lib/host_context/async_value_ref.cc",
        "lib/host_context/concurrent_work_queue.cc",
        "lib/host_context/diagnostic.cc",
        "lib/host_context/host_allocator.cc",
        "lib/host_context/host_buffer.cc",
        "lib/host_context/host_context.cc",
        "lib/host_context/host_context_ptr.cc",
        "lib/host_context/kernel_frame.cc",
        "lib/host_context/kernel_registry.cc",
        "lib/host_context/native_function.cc",
        "lib/host_context/parallel_for.cc",
        "lib/host_context/profiled_allocator.cc",
        "lib/host_context/shared_context.cc",
        "lib/host_context/single_threaded_work_queue.cc",
        "lib/host_context/test_fixed_size_allocator.cc",
        "@tf_runtime//third_party/concurrent_work_queue:concurrent_work_queue_srcs",
    ],
    hdrs = [
        "include/tfrt/host_context/async_value.h",
        "include/tfrt/host_context/async_value_ref.h",
        "include/tfrt/host_context/attribute_utils.h",
        "include/tfrt/host_context/chain.h",
        "include/tfrt/host_context/concurrent_work_queue.h",
        "include/tfrt/host_context/diagnostic.h",
        "include/tfrt/host_context/execution_context.h",
        "include/tfrt/host_context/function.h",
        "include/tfrt/host_context/host_allocator.h",
        "include/tfrt/host_context/host_buffer.h",
        "include/tfrt/host_context/host_context.h",
        "include/tfrt/host_context/host_context_ptr.h",
        "include/tfrt/host_context/kernel_frame.h",
        "include/tfrt/host_context/kernel_registry.h",
        "include/tfrt/host_context/kernel_utils.h",
        "include/tfrt/host_context/location.h",
        "include/tfrt/host_context/native_function.h",
        "include/tfrt/host_context/parallel_for.h",
        "include/tfrt/host_context/shared_context.h",
        "include/tfrt/host_context/task_function.h",
        "include/tfrt/host_context/type_name.h",
    ],
    alwayslink_static_registration_src = "lib/host_context/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":support",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "support",
    srcs = [
        "lib/support/alloc.cc",
        "lib/support/hash_util.cc",
        "lib/support/logging.cc",
        "lib/support/ref_count.cc",
        "lib/support/stack_trace.cc",
        "lib/support/string_util.cc",
    ],
    hdrs = [
        "include/tfrt/support/aligned_buffer.h",
        "include/tfrt/support/alloc.h",
        "include/tfrt/support/bef_encoding.h",
        "include/tfrt/support/bef_reader.h",
        "include/tfrt/support/byte_order.h",
        "include/tfrt/support/compiler_annotations.h",
        "include/tfrt/support/concurrent_vector.h",
        "include/tfrt/support/error_util.h",
        "include/tfrt/support/forward_decls.h",
        "include/tfrt/support/fp16.h",
        "include/tfrt/support/hash_util.h",
        "include/tfrt/support/latch.h",
        "include/tfrt/support/logging.h",
        "include/tfrt/support/msan.h",
        "include/tfrt/support/mutex.h",
        "include/tfrt/support/op_registry_impl.h",
        "include/tfrt/support/ostream.h",
        "include/tfrt/support/rc_array.h",
        "include/tfrt/support/ref_count.h",
        "include/tfrt/support/string_util.h",
        "include/tfrt/support/template_util.h",
        "include/tfrt/support/thread_annotations.h",
        "include/tfrt/support/type_traits.h",
    ],
    visibility = [":friends"],
    deps = [
        "@llvm-project//llvm:support",
        "@tf_runtime//third_party/llvm_derived:ostream",
    ],
)

tfrt_cc_library(
    name = "tracing",
    srcs = [
        "lib/tracing/tracing.cc",
    ],
    hdrs = [
        "include/tfrt/tracing/tracing.h",
    ],
    visibility = [":friends"],
    deps = [
        ":support",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "simple_tracing_sink",
    srcs = [
        "lib/tracing/simple_tracing_sink/simple_tracing_sink.cc",
    ],
    hdrs = [
        "include/tfrt/tracing/simple_tracing_sink/simple_tracing_sink.h",
    ],
    alwayslink_static_registration_src =
        "lib/tracing/simple_tracing_sink/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":support",
        ":tracing",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "befexecutor",
    srcs = [
        "lib/bef_executor/bef_executor.cc",
        "lib/bef_executor/bef_file.cc",
        "lib/bef_executor/bef_file_impl.h",
    ],
    hdrs = [
        "include/tfrt/bef_executor/bef_file.h",
        "include/tfrt/support/bef_encoding.h",
    ],
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        ":tracing",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "metrics_api",
    srcs = [
        "lib/metrics/metrics_api_dummy.cc",
    ],
    hdrs = [
        "include/tfrt/metrics/metrics_api.h",
    ],
    visibility = [":friends"],
    deps = [
        ":support",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "tensor",
    srcs = [
        "lib/tensor/btf.cc",
        "lib/tensor/coo_host_tensor.cc",
        "lib/tensor/coo_host_tensor_kernels.cc",
        "lib/tensor/dense_host_tensor.cc",
        "lib/tensor/dense_host_tensor_kernels.cc",
        "lib/tensor/dtype.cc",
        "lib/tensor/scalar_host_tensor.cc",
        "lib/tensor/string_host_tensor.cc",
        "lib/tensor/string_host_tensor_kernels.cc",
        "lib/tensor/tensor.cc",
        "lib/tensor/tensor_serialize_utils.cc",
        "lib/tensor/tensor_shape.cc",
        "lib/tensor/tensor_shape_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/tensor/btf.h",
        "include/tfrt/tensor/btf_reader_util.h",
        "include/tfrt/tensor/coo_host_tensor.h",
        "include/tfrt/tensor/dense_host_tensor.h",
        "include/tfrt/tensor/dense_host_tensor_kernels.h",
        "include/tfrt/tensor/dense_host_tensor_view.h",
        "include/tfrt/tensor/dense_tensor_utils.h",
        "include/tfrt/tensor/dense_view.h",
        "include/tfrt/tensor/dtype.def",
        "include/tfrt/tensor/dtype.h",
        "include/tfrt/tensor/host_tensor.h",
        "include/tfrt/tensor/scalar_host_tensor.h",
        "include/tfrt/tensor/string_host_tensor.h",
        "include/tfrt/tensor/string_host_tensor_kernels.h",
        "include/tfrt/tensor/tensor.h",
        "include/tfrt/tensor/tensor_metadata.h",
        "include/tfrt/tensor/tensor_serialize_utils.h",
        "include/tfrt/tensor/tensor_shape.h",
    ],
    alwayslink_static_registration_src = "lib/tensor/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:Support",
    ],
)

tfrt_cc_library(
    name = "basic_kernels",
    srcs = [
        "lib/basic_kernels/boolean_kernels.cc",
        "lib/basic_kernels/control_flow_kernels.cc",
        "lib/basic_kernels/float_kernels.cc",
        "lib/basic_kernels/integer_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/basic_kernels/basic_kernels.h",
    ],
    alwayslink_static_registration_src = "lib/basic_kernels/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "mlirtobef",
    srcs = [
        "lib/bef_converter/mlir_to_bef/mlir_to_bef.cc",
    ],
    hdrs = [
        "include/tfrt/bef_converter/mlir_to_bef.h",
    ],
    alwayslink_static_registration_src = "lib/bef_converter/mlir_to_bef/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":core_runtime_opdefs",
        ":support",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Translation",
    ],
)

tfrt_cc_library(
    name = "beftomlir",
    srcs = ["lib/bef_converter/bef_to_mlir/bef_to_mlir.cc"],
    hdrs = ["include/tfrt/bef_converter/bef_to_mlir.h"],
    alwayslink_static_registration_src = "lib/bef_converter/bef_to_mlir/static_registration.cc",
    deps = [
        ":core_runtime_opdefs",
        ":support",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Parser",
        "@llvm-project//mlir:Support",
        "@llvm-project//mlir:Translation",
    ],
)

filegroup(
    name = "OpBaseTdFiles",
    srcs = [
        "include/tfrt/tensor/opdefs/tensor_shape_base.td",
        "include/tfrt/tfrt_op_base.td",
        "@llvm-project//mlir:include/mlir/IR/OpBase.td",
    ],
    visibility = [":friends"],
)

exports_files(
    ["include/tfrt/tfrt_op_base.td"],
    visibility = [":friends"],
)

gentbl(
    name = "basic_kernels_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/basic_kernels/opdefs/basic_kernels.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/basic_kernels/opdefs/basic_kernels_opdefs.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/basic_kernels/opdefs/basic_kernels.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
        "@llvm-project//mlir:include/mlir/Interfaces/SideEffects.td",
    ],
)

tfrt_cc_library(
    name = "basic_kernels_opdefs",
    srcs = [
        "lib/basic_kernels/opdefs/basic_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/basic_kernels/opdefs/basic_kernels.h",
    ],
    alwayslink_static_registration_src = "lib/basic_kernels/opdefs/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":basic_kernels_opdefs_inc_gen",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:SideEffects",
        "@llvm-project//mlir:Support",
    ],
)

gentbl(
    name = "tensor_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/tensor/opdefs/tensor_shape.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/tensor/opdefs/tensor_shape.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/tensor/opdefs/tensor_shape.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
        "@llvm-project//mlir:include/mlir/Interfaces/SideEffects.td",
    ],
)

gentbl(
    name = "dense_host_tensor_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/tensor/opdefs/dense_host_tensor.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/tensor/opdefs/dense_host_tensor.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/tensor/opdefs/dense_host_tensor.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
        "@llvm-project//mlir:include/mlir/Interfaces/SideEffects.td",
    ],
)

gentbl(
    name = "coo_host_tensor_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/tensor/opdefs/coo_host_tensor.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/tensor/opdefs/coo_host_tensor.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/tensor/opdefs/coo_host_tensor.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
    ],
)

tfrt_cc_library(
    name = "tensor_opdefs",
    srcs = [
        "lib/tensor/opdefs/coo_host_tensor.cc",
        "lib/tensor/opdefs/dense_host_tensor.cc",
        "lib/tensor/opdefs/tensor_shape.cc",
    ],
    hdrs = [
        "include/tfrt/tensor/opdefs/coo_host_tensor.h",
        "include/tfrt/tensor/opdefs/dense_host_tensor.h",
        "include/tfrt/tensor/opdefs/tensor_shape.h",
    ],
    alwayslink_static_registration_src = "lib/tensor/opdefs/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":coo_host_tensor_opdefs_inc_gen",
        ":dense_host_tensor_opdefs_inc_gen",
        ":tensor_opdefs_inc_gen",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:SideEffects",
    ],
)

gentbl(
    name = "core_runtime_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/core_runtime/opdefs/core_runtime_opdefs.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/core_runtime/opdefs/core_runtime_opdefs.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/core_runtime/opdefs/core_runtime.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
        "@llvm-project//mlir:include/mlir/Interfaces/SideEffects.td",
    ],
)

tfrt_cc_library(
    name = "core_runtime_opdefs",
    srcs = [
        "lib/core_runtime/opdefs/core_runtime.cc",
    ],
    hdrs = [
        "include/tfrt/core_runtime/opdefs/attributes.h",
        "include/tfrt/core_runtime/opdefs/core_runtime.h",
    ],
    alwayslink_static_registration_src = "lib/core_runtime/opdefs/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":core_runtime_opdefs_inc_gen",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:SideEffects",
        "@llvm-project//mlir:Support",
    ],
)

tfrt_cc_library(
    name = "bef_executor_driver",
    srcs = [
        "lib/bef_executor_driver/bef_executor_driver.cc",
    ],
    hdrs = [
        "include/tfrt/bef_executor_driver/bef_executor_driver.h",
    ],
    visibility = [":friends"],
    deps = [
        ":befexecutor",
        ":core_runtime",
        ":hostcontext",
        ":metrics_api",
        ":support",
        ":tracing",
        "@llvm-project//llvm:support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
    ],
)

tfrt_cc_library(
    name = "core_runtime",
    srcs = [
        "lib/core_runtime/composite_op_handler.cc",
        "lib/core_runtime/composite_op_kernels.cc",
        "lib/core_runtime/core_runtime.cc",
        "lib/core_runtime/core_runtime_op.cc",
        "lib/core_runtime/dispatch_utils.cc",
        "lib/core_runtime/kernels.cc",
        "lib/core_runtime/logging_op_handler.cc",
        "lib/core_runtime/op_attrs.cc",
        "lib/core_runtime/tensor_handle.cc",
        "lib/core_runtime/test_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/core_runtime/core_runtime.h",
        "include/tfrt/core_runtime/core_runtime_op.h",
        "include/tfrt/core_runtime/dispatch_utils.h",
        "include/tfrt/core_runtime/kernels.h",
        "include/tfrt/core_runtime/op_args.h",
        "include/tfrt/core_runtime/op_attr_type.def",
        "include/tfrt/core_runtime/op_attr_type.h",
        "include/tfrt/core_runtime/op_attrs.h",
        "include/tfrt/core_runtime/op_handler.h",
        "include/tfrt/core_runtime/op_handler_factory.h",
        "include/tfrt/core_runtime/op_invocation.h",
        "include/tfrt/core_runtime/op_metadata_function.h",
        "include/tfrt/core_runtime/op_utils.h",
        "include/tfrt/core_runtime/tensor_handle.h",
        "lib/core_runtime/composite_op_handler.h",
        "lib/core_runtime/logging_op_handler.h",
    ],
    alwayslink_static_registration_src = "lib/core_runtime/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        ":tensor",
        ":tracing",
        "@llvm-project//llvm:support",
    ],
)

tfrt_cc_library(
    name = "test_kernels",
    srcs = [
        "lib/test_kernels/async_kernels.cc",
        "lib/test_kernels/async_test_kernels.cc",
        "lib/test_kernels/atomic_test_kernels.cc",
        "lib/test_kernels/benchmark_kernels.cc",
        "lib/test_kernels/simple_kernels.cc",
        "lib/test_kernels/simple_test_kernels.cc",
        "lib/test_kernels/test_native_functions.cc",
        "lib/test_kernels/tutorial_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/test_kernels.h",
    ],
    alwayslink_static_registration_src = "lib/test_kernels/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        ":tensor",
        "@llvm-project//llvm:support",
    ],
)

gentbl(
    name = "test_kernels_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/test_kernels/opdefs/test_kernels.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/test_kernels/opdefs/test_kernels_opdefs.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/test_kernels/opdefs/test_kernels.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
        "@llvm-project//mlir:include/mlir/Interfaces/SideEffects.td",
    ],
)

tfrt_cc_library(
    name = "test_kernels_opdefs",
    srcs = [
        "lib/test_kernels/opdefs/test_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/test_kernels/opdefs/test_kernels.h",
    ],
    alwayslink_static_registration_src = "lib/test_kernels/opdefs/static_registration.cc",
    deps = [
        ":test_kernels_opdefs_inc_gen",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:SideEffects",
        "@llvm-project//mlir:Support",
    ],
)

tfrt_cc_library(
    name = "data",
    srcs = [
        "lib/data/batch_dataset.h",
        "lib/data/data_kernels.cc",
        "lib/data/dataset.h",
        "lib/data/interleave_dataset.h",
        "lib/data/io.h",
        "lib/data/map_dataset.h",
        "lib/data/prefetch_dataset.h",
        "lib/data/range_dataset.h",
        "lib/data/repeat_dataset.h",
        "lib/data/slice_dataset.h",
        "lib/data/tf_record_dataset.cc",
        "lib/data/tf_record_dataset.h",
    ],
    alwayslink_static_registration_src = "lib/data/static_registration.cc",
    visibility = [":friends"],
    deps = [
        ":hostcontext",
        ":support",
        ":tensor",
        "@llvm-project//llvm:support",
    ],
)

gentbl(
    name = "data_opdefs_inc_gen",
    tbl_outs = [
        (
            "-gen-op-decls",
            "include/tfrt/data/opdefs/data_kernels.h.inc",
        ),
        (
            "-gen-op-defs",
            "include/tfrt/data/opdefs/data_kernels_opdefs.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "include/tfrt/data/opdefs/data_kernels.td",
    td_includes = ["include"],
    td_srcs = [
        ":OpBaseTdFiles",
    ],
)

tfrt_cc_library(
    name = "data_opdefs",
    srcs = [
        "lib/data/opdefs/data_kernels.cc",
    ],
    hdrs = [
        "include/tfrt/data/opdefs/data_kernels.h",
    ],
    alwayslink_static_registration_src = "lib/data/opdefs/static_registration.cc",
    deps = [
        ":data_opdefs_inc_gen",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
    ],
)
