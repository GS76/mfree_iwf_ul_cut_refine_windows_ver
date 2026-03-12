cmake_minimum_required(VERSION 3.14)
project(mfree_iwf_ul_cut_refine_windows_ver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

find_package(OpenMP)

include_directories(src)
include_directories(src/benchmarks)
include_directories(${glm_SOURCE_DIR})

file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/tests/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*test_omp_scaling\\.cpp$")

add_executable(mfree_iwf_ul_cut_refine_windows_ver ${SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(mfree_iwf_ul_cut_refine_windows_ver PUBLIC OpenMP::OpenMP_CXX)
    
    # Add validation target
    add_executable(validate_omp src/benchmarks/test_omp_scaling.cpp)
    target_link_libraries(validate_omp PUBLIC OpenMP::OpenMP_CXX)
endif()
