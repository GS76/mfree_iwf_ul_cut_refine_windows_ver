cmake_minimum_required(VERSION 3.16)

project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(mfree_iwf
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/refine_cut_main.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/test_benches.cpp
  src/benchmarks/test_cuttings.cpp
  src/benchmarks/test_density.cpp
)

target_include_directories(mfree_iwf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf PRIVATE OpenMP::OpenMP_CXX)
endif()

enable_testing()

add_executable(mfree_iwf_validate
  src/adaptivity.cpp
  src/body.cpp
  src/cont_mech.cpp
  src/contact.cpp
  src/correctors.cpp
  src/derivatives.cpp
  src/fe_tool.cpp
  src/grid.cpp
  src/johnson_cook_Sima_2010.cpp
  src/kernel.cpp
  src/leap_frog.cpp
  src/logger.cpp
  src/material.cpp
  src/particle.cpp
  src/plasticity.cpp
  src/precomp_shape_functions.cpp
  src/simulation_data.cpp
  src/simulation_time.cpp
  src/thermal.cpp
  src/tool.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/validate_main.cpp
)

target_include_directories(mfree_iwf_validate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf_validate PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(NAME mfree_iwf_validate COMMAND mfree_iwf_validate)
