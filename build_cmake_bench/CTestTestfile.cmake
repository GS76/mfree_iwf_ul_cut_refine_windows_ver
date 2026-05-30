# CMake generated Testfile for 
# Source directory: D:/mfree_iwf_ul_cut_refine_windows_ver
# Build directory: D:/mfree_iwf_ul_cut_refine_windows_ver/build_cmake_bench
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_property_interpolation "D:/mfree_iwf_ul_cut_refine_windows_ver/build_cmake_bench/test_property_interpolation.exe")
set_tests_properties(test_property_interpolation PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;134;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
add_test(smoke_model_1 "D:/mfree_iwf_ul_cut_refine_windows_ver/build_cmake_bench/mfree_iwf.exe" "--smoke" "-m" "1")
set_tests_properties(smoke_model_1 PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;136;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
subdirs("_deps/glm-build")
