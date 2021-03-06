# ******************************************************************************
# Copyright 2017-2019 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

include(FindOpenMP)

set(SRC
    cpu_backend.cpp
    cpu_builder.cpp
    cpu_call_frame.cpp
    cpu_executor.cpp
    cpu_external_function.cpp
    cpu_kernels.cpp
    cpu_layout_descriptor.cpp
    cpu_op_annotations.cpp
    cpu_tensor_view_wrapper.cpp
    cpu_tensor_view.cpp
    cpu_tracing.cpp
    cpu_visualize_tree.cpp
    cpu_cse.cpp
    cpu_debugger.cpp
    builder/add.cpp
    builder/allreduce.cpp
    builder/avg_pool.cpp
    builder/argmin.cpp
    builder/argmax.cpp
    builder/batch_norm.cpp
    builder/broadcast.cpp
    builder/bounded_relu.cpp
    builder/concat.cpp
    builder/convert.cpp
    builder/convert_layout.cpp
    builder/convolution.cpp
    builder/dot.cpp
    builder/embedding_lookup.cpp
    builder/function_call.cpp
    builder/leaky_relu.cpp
    builder/lstm.cpp
    builder/lrn.cpp
    builder/matmul_bias.cpp
    builder/max.cpp
    builder/max_pool.cpp
    builder/min.cpp
    builder/one_hot.cpp
    builder/relu.cpp
    builder/pad.cpp
    builder/product.cpp
    builder/reduce_function.cpp
    builder/reduce_function_window.cpp
    builder/replace_slice.cpp
    builder/quantization.cpp
    builder/quantized_avg_pool.cpp
    builder/quantized_conv.cpp
    builder/quantized_max_pool.cpp
    builder/reshape.cpp
    builder/reverse.cpp
    builder/reverse_sequence.cpp
    builder/rnn.cpp
    builder/select.cpp
    builder/select_and_scatter.cpp
    builder/sigmoid.cpp
    builder/slice.cpp
    builder/state.cpp
    builder/softmax.cpp
    builder/get_output_element.cpp
    builder/sum.cpp
    builder/topk.cpp
    builder/update_slice.cpp
    kernel/pad.cpp
    kernel/reduce_max.cpp
    kernel/reduce_sum.cpp
    kernel/reshape.cpp
    mkldnn_emitter.cpp
    mkldnn_invoke.cpp
    mkldnn_utils.cpp
    op/batch_dot.cpp
    op/batch_norm_relu.cpp
    op/bounded_relu.cpp
    op/conv_add.cpp
    op/conv_bias.cpp
    op/conv_relu.cpp
    op/convert_layout.cpp
    op/group_conv.cpp
    op/group_conv_bias.cpp
    op/halide_op.cpp
    op/leaky_relu.cpp
    op/loop_kernel.cpp
    op/lstm.cpp
    op/matmul_bias.cpp
    op/max_pool_with_indices.cpp
    op/rnn.cpp
    op/sigmoid_mul.cpp
    op/update_slice.cpp
    pass/cpu_assignment.cpp
    pass/cpu_collapse_dims.cpp
    pass/cpu_fusion.cpp
    pass/cpu_horizontal_fusion.cpp
    pass/cpu_layout.cpp
    pass/cpu_loop_kernel_fusion.cpp
    pass/cpu_mat_fusion.cpp
    pass/cpu_memory_optimization.cpp
    pass/cpu_post_layout_optimizations.cpp
    pass/cpu_rnn_fusion.cpp
    pass/cpu_workspace_insertion.cpp
    ngraph_version.cpp
)

if (NOT NGRAPH_DEX_ONLY)
    set(SRC
        ${SRC}
        cpu_emitter.cpp
        cpu_kernel_emitters.cpp
        cpu_kernel_utils.cpp
        )
endif()

if (NGRAPH_HALIDE)
    set(SRC
        ${SRC}
        builder/halide_op.cpp
        builder/loop_kernel.cpp
        builder/halide_generators.cpp
        pass/halide_subgraph_extraction.cpp
        )
endif()

if (NGRAPH_TBB_ENABLE)
    include(${TBB_ROOT}/cmake/TBBBuild.cmake)
    tbb_build(TBB_ROOT ${TBB_ROOT} MAKE_ARGS tbb_build_dir=${CMAKE_CURRENT_BINARY_DIR}/tbb_build
                tbb_build_prefix=tbb CONFIG_DIR TBB_DIR)
    find_package(TBB REQUIRED tbb)
    if (NOT TBB_FOUND)
        message(FATAL_ERROR "TBB is needed by the CPU backend and was not found")
    else()
        message(STATUS "Found TBB and imported target ${TBB_IMPORTED_TARGETS}")
    endif()

    set_source_files_properties(cpu_external_function.cpp
        PROPERTIES COMPILE_DEFINITIONS "NGRAPH_TBB_ENABLE")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(TBB_LIB_NAME tbb_debug)
        set(TBB_BUILDDIR_NAME tbb_debug)
    else()
        set(TBB_LIB_NAME tbb)
        set(TBB_BUILDDIR_NAME tbb_release)
    endif()
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tbb_build/${TBB_BUILDDIR_NAME}/
        DESTINATION ${NGRAPH_INSTALL_LIB}
        FILES_MATCHING REGEX "/${CMAKE_SHARED_LIBRARY_PREFIX}${TBB_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}(\\.[0-9]+)*$"
    )
    add_library(libtbb INTERFACE)
    target_link_libraries(libtbb INTERFACE
        ${CMAKE_CURRENT_BINARY_DIR}/tbb_build/${TBB_BUILDDIR_NAME}/${CMAKE_SHARED_LIBRARY_PREFIX}${TBB_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
    target_include_directories(libtbb SYSTEM INTERFACE ${TBB_ROOT}/include)
endif()

if (NGRAPH_CPU_ENABLE)
    set(NGRAPH_CPU_DEBUGINFO_ENABLE 0 CACHE STRING "Enable debuginfo in the CPU backend")

    add_library(cpu_backend SHARED ${SRC})
    if(NGRAPH_LIB_VERSIONING_ENABLE)
        set_target_properties(cpu_backend PROPERTIES
            VERSION ${NGRAPH_VERSION}
            SOVERSION ${NGRAPH_API_VERSION})
    endif()
    if (NGRAPH_DEX_ONLY)
        target_compile_definitions(cpu_backend PRIVATE "NGRAPH_DEX_ONLY")
    endif()
    if (NGRAPH_HALIDE)
        target_compile_definitions(cpu_backend PRIVATE "NGRAPH_HALIDE")
        ExternalProject_Get_Property(ext_halide BINARY_DIR)
        target_include_directories(cpu_backend SYSTEM PRIVATE ${BINARY_DIR}/include)
        target_link_libraries(cpu_backend PRIVATE ${BINARY_DIR}/lib/libHalide.a libhalidellvm)
        add_dependencies(cpu_backend ext_halide)
    endif()

    if(OPENMP_FOUND)
        target_compile_options(cpu_backend PRIVATE "${OpenMP_CXX_FLAGS}")
        target_compile_definitions(cpu_backend PRIVATE EIGEN_OPENMP)
    else()
         message(WARNING "The build toolset doesn't support OpenMP. This will impact performance and lead to slowdowns.")
    endif()

    if(NGRAPH_DISTRIBUTED_ENABLE)
        target_compile_definitions(cpu_backend PRIVATE NGRAPH_DISTRIBUTED)
        target_include_directories(cpu_backend SYSTEM PRIVATE libmlsl)
        target_link_libraries(cpu_backend PRIVATE libmlsl)
    endif()

    add_dependencies(cpu_backend ext_mkldnn ext_eigen)
    target_link_libraries(cpu_backend PUBLIC ngraph libmkldnn libeigen libjson libtbb)
    if (NOT NGRAPH_DEX_ONLY)
        target_link_libraries(cpu_backend PUBLIC codegen)
    endif()
    target_include_directories(cpu_backend SYSTEM PUBLIC libmkldnn)
    set_target_properties(cpu_backend PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${NGRAPH_BUILD_DIR})
    if (NOT APPLE)
        # CPU backend uses third-party libraries like Eigen that might be linked in and
        # exported by other DSOs as well. In the absence of versioning, this could lead to the
        # CPU backend picking up the wrong version or even multiple versions of the
        # third-party library. -Bsymbolic-functions tells the linker to prefer the internal
        # version inside cpu_backend over what is available through the global symbol table
        set_property(TARGET cpu_backend APPEND PROPERTY LINK_FLAGS "-Wl,-Bsymbolic-functions -Wl,--exclude-libs=ALL")
    endif()

    install(TARGETS cpu_backend LIBRARY DESTINATION ${NGRAPH_INSTALL_LIB})
endif()
