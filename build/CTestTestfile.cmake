# CMake generated Testfile for 
# Source directory: D:/mfree_iwf_ul_cut_refine_windows_ver
# Build directory: D:/mfree_iwf_ul_cut_refine_windows_ver/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_property_interpolation "D:/mfree_iwf_ul_cut_refine_windows_ver/build/test_property_interpolation.exe")
set_tests_properties(test_property_interpolation PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;178;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
add_test(smoke_model_1 "D:/mfree_iwf_ul_cut_refine_windows_ver/build/mfree_iwf.exe" "--smoke" "-m" "1")
set_tests_properties(smoke_model_1 PROPERTIES  TIMEOUT "120" _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;180;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
add_test(smoke_config_model1 "D:/mfree_iwf_ul_cut_refine_windows_ver/build/mfree_iwf.exe" "--smoke" "--config" "D:/mfree_iwf_ul_cut_refine_windows_ver/configs/model1.json")
set_tests_properties(smoke_config_model1 PROPERTIES  TIMEOUT "120" _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;183;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
add_test(test_json_unicode "D:/mfree_iwf_ul_cut_refine_windows_ver/build/test_json_unicode.exe")
set_tests_properties(test_json_unicode PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;195;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
subdirs("_deps/glm-build")
