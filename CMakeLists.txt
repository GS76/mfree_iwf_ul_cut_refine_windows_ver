cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(mfree_iwf_ul_cut_refine LANGUAGES CXX)

option(MFREE_ENABLE_BENCHMARKS "Build benchmark executables" ON)
option(MFREE_ENABLE_TESTS "Build unit tests" ON)
option(MFREE_USE_FETCHCONTENT_GLM "Fetch GLM via FetchContent" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

include(FetchContent)

if(MFREE_USE_FETCHCONTENT_GLM)
	FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		GIT_TAG 0.9.9.8
	)
	FetchContent_MakeAvailable(glm)
else()
	find_package(glm CONFIG REQUIRED)
endif()

find_package(OpenMP REQUIRED)

function(mfree_apply_target_defaults target_name)
	target_compile_features(${target_name} PRIVATE cxx_std_17)

	if(MSVC)
		target_compile_options(${target_name} PRIVATE
			$<$<CONFIG:Debug>:/Od /Zi>
			$<$<CONFIG:Release>:/O2>
		)
	else()
		target_compile_options(${target_name} PRIVATE
			$<$<CONFIG:Debug>:-O0 -g>
			$<$<CONFIG:Release>:-O3>
		)
	endif()

	target_compile_definitions(${target_name} PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
		$<$<CONFIG:Release>:NDEBUG>
	)

	if(WIN32)
		target_compile_definitions(${target_name} PRIVATE _USE_MATH_DEFINES)
	endif()
endfunction()

function(mfree_apply_openmp_flags_if_needed target_name)
	if(MSVC)
		target_link_libraries(${target_name} PRIVATE OpenMP::OpenMP_CXX)
		return()
	endif()
	if(NOT OpenMP_CXX_FLAGS)
		return()
	endif()
	separate_arguments(_mfree_omp_flags NATIVE_COMMAND "${OpenMP_CXX_FLAGS}")
	target_link_options(${target_name} PRIVATE ${_mfree_omp_flags})
endfunction()

set(MFREE_CORE_LIB_SOURCES
	src/adaptivity.cpp
	src/body.cpp
	src/cont_mech.cpp
	src/contact.cpp
	src/correctors.cpp
	src/derivatives.cpp
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
)

set(MFREE_MAIN_SOURCE
	src/refine_cut_main.cpp
)

set(MFREE_BENCH_SOURCES
	src/benchmarks/material_library.cpp
	src/benchmarks/test_benches.cpp
	src/benchmarks/test_cuttings.cpp
	src/benchmarks/test_density.cpp
)

add_library(mfree_core STATIC
	${MFREE_CORE_LIB_SOURCES}
)
target_include_directories(mfree_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(mfree_core PUBLIC glm::glm OpenMP::OpenMP_CXX)
mfree_apply_target_defaults(mfree_core)

add_library(mfree_benchmark_support STATIC
	${MFREE_BENCH_SOURCES}
)
target_include_directories(mfree_benchmark_support PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)
target_link_libraries(mfree_benchmark_support PUBLIC mfree_core)
mfree_apply_target_defaults(mfree_benchmark_support)

add_executable(mfree_iwf
	${MFREE_MAIN_SOURCE}
)

target_include_directories(mfree_iwf PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
)

target_link_libraries(mfree_iwf PRIVATE mfree_benchmark_support OpenMP::OpenMP_CXX)
if(MSVC)
	set_property(TARGET mfree_iwf PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set_property(TARGET mfree_iwf PROPERTY OUTPUT_NAME "mfree_iwf")
mfree_apply_target_defaults(mfree_iwf)
mfree_apply_openmp_flags_if_needed(mfree_iwf)

add_executable(mfree_iwf_ul_cut_refine_windows_ver ALIAS mfree_iwf)

if(MFREE_ENABLE_BENCHMARKS)
	add_executable(validate_omp src/benchmarks/test_omp_scaling.cpp)
	target_include_directories(validate_omp PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks
	)
	target_link_libraries(validate_omp PRIVATE OpenMP::OpenMP_CXX)
	mfree_apply_target_defaults(validate_omp)
	mfree_apply_openmp_flags_if_needed(validate_omp)
endif()

if(MFREE_ENABLE_TESTS)
	enable_testing()

	add_executable(test_property_interpolation
		src/tests/test_property_interpolation.cpp
	)
	target_include_directories(test_property_interpolation PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
	)
	target_link_libraries(test_property_interpolation PRIVATE mfree_core)
	mfree_apply_target_defaults(test_property_interpolation)
	mfree_apply_openmp_flags_if_needed(test_property_interpolation)
	add_test(NAME test_property_interpolation COMMAND test_property_interpolation)

	add_test(NAME smoke_model_1 COMMAND mfree_iwf --smoke -m 1)
	set_tests_properties(smoke_model_1 PROPERTIES TIMEOUT 30)
endif()

include(GNUInstallDirs)
install(TARGETS mfree_iwf
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(MFREE_ENABLE_BENCHMARKS)
	install(TARGETS validate_omp
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

set(CPACK_PACKAGE_NAME "mfree_iwf_ul_cut_refine")
set(CPACK_PACKAGE_VENDOR "IWF / ETH Zurich")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
