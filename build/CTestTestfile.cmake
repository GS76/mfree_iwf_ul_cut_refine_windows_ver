# CMake generated Testfile for 
# Source directory: D:/mfree_iwf_ul_cut_refine_windows_ver
# Build directory: D:/mfree_iwf_ul_cut_refine_windows_ver/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(mfree_iwf_validate "D:/mfree_iwf_ul_cut_refine_windows_ver/build/Debug/mfree_iwf_validate.exe")
  set_tests_properties(mfree_iwf_validate PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;85;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(mfree_iwf_validate "D:/mfree_iwf_ul_cut_refine_windows_ver/build/Release/mfree_iwf_validate.exe")
  set_tests_properties(mfree_iwf_validate PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;85;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(mfree_iwf_validate "D:/mfree_iwf_ul_cut_refine_windows_ver/build/MinSizeRel/mfree_iwf_validate.exe")
  set_tests_properties(mfree_iwf_validate PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;85;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(mfree_iwf_validate "D:/mfree_iwf_ul_cut_refine_windows_ver/build/RelWithDebInfo/mfree_iwf_validate.exe")
  set_tests_properties(mfree_iwf_validate PROPERTIES  _BACKTRACE_TRIPLES "D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;85;add_test;D:/mfree_iwf_ul_cut_refine_windows_ver/CMakeLists.txt;0;")
else()
  add_test(mfree_iwf_validate NOT_AVAILABLE)
endif()
