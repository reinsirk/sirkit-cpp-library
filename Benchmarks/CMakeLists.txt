# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required(VERSION 3.24)
project(sirkit_benchmarks LANGUAGES CXX)

option(SIRKIT_BENCHMARK_SKIP_RUNTIME_CHECKS "Disable sirkit input checks in benchmarks" OFF)

include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BENCHMARK_ENABLE_WERROR OFF)
# Google Benchmark (Apache-2.0) https://github.com/google/benchmark/tree/v1.10.0
FetchContent_Declare(google_benchmark
    URL https://codeload.github.com/google/benchmark/tar.gz/refs/tags/v1.10.0
    URL_HASH SHA256=acc5c7c0fbed855c4aaa2d7c3c36fb82b0c15c28f4e151e3f679c2fc0aa404af
    TLS_VERIFY ON)
FetchContent_MakeAvailable(google_benchmark)

set(lca_source "${CMAKE_CURRENT_SOURCE_DIR}/../Graph/LCA.cpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${lca_source}")
file(SHA256 "${lca_source}" lca_sha256)

add_executable(sirkit_benchmarks lca.cpp)
target_compile_features(sirkit_benchmarks PRIVATE cxx_std_23)
set_target_properties(sirkit_benchmarks PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(sirkit_benchmarks PRIVATE
    SIRKIT_BENCHMARK_LCA_SHA256="${lca_sha256}"
    SIRKIT_BENCHMARK_COMPILER="${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
    SIRKIT_BENCHMARK_BUILD_TYPE="$<CONFIG>")
if(SIRKIT_BENCHMARK_SKIP_RUNTIME_CHECKS)
    target_compile_definitions(sirkit_benchmarks PRIVATE SKIP_RUNTIME_CHECKS)
endif()
target_link_libraries(sirkit_benchmarks PRIVATE benchmark::benchmark)
