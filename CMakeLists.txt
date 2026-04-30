cmake_minimum_required(VERSION 3.16)

project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MFREE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors for project targets" OFF)

function(mfree_apply_warning_options target_name)
  if(MSVC)
    target_compile_definitions(${target_name} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${target_name} PRIVATE /W4 /wd4100 /wd4189 /wd4244 /wd4267 /wd4305 /wd4456 /wd4458 /wd4505)
    if(MFREE_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    if(MFREE_WARNINGS_AS_ERRORS)
      target_compile_options(${target_name} PRIVATE -Werror)
    endif()
  endif()
endfunction()

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
  src/timestep_estimator.cpp
  src/tool_adapter_poly.cpp
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

mfree_apply_warning_options(mfree_iwf)

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
  src/timestep_estimator.cpp
  src/tool_adapter_poly.cpp
  src/vtk_writer.cpp
  src/benchmarks/material_library.cpp
  src/benchmarks/validate_main.cpp
)

target_include_directories(mfree_iwf_validate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

mfree_apply_warning_options(mfree_iwf_validate)

if(OpenMP_CXX_FOUND)
  target_link_libraries(mfree_iwf_validate PRIVATE OpenMP::OpenMP_CXX)
endif()

add_test(NAME mfree_iwf_validate COMMAND mfree_iwf_validate)

add_test(NAME mfree_iwf_preprocess_model1_rigid COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_rigid PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_rigid"
)

add_test(NAME mfree_iwf_preprocess_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_preprocess_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh"
)

add_test(NAME mfree_iwf_geom_validate_model1_fe_only COMMAND $<TARGET_FILE:mfree_iwf> -m 1)
set_tests_properties(mfree_iwf_geom_validate_model1_fe_only PROPERTIES
  ENVIRONMENT
    "MFREE_PREPROCESS_ONLY=1;MFREE_CLEAN_RESULTS=1;MFREE_RESULTS_DIR=${CMAKE_BINARY_DIR}/ctest_results/geom_model1_fe_only;MFREE_NO_RIGID_TOOL=1;MFREE_USE_FE_TOOL_FOR_CONTACT=1;MFREE_CONTACT_MU=0.35;MFREE_FE_TOOL_MSH=${CMAKE_SOURCE_DIR}/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh;MFREE_GEOM_VALIDATE=1"
)

add_executable(mfree_geom_validation_math_test
  src/benchmarks/test_geom_validation_math.cpp
)

target_include_directories(mfree_geom_validation_math_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

mfree_apply_warning_options(mfree_geom_validation_math_test)

add_test(NAME mfree_geom_validation_math_test COMMAND mfree_geom_validation_math_test)

add_executable(mfree_extract_final_chip
  src/tools/extract_final_chip.cpp
)

mfree_apply_warning_options(mfree_extract_final_chip)
