# CMake generated Testfile for 
# Source directory: C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests
# Build directory: C:/Users/a/Documents/nx_open_integrations/cpp/vms_server_plugins/opencv_object_detection_analytics_plugin/step5/build/nx_kit/unit_tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(nx_kit_ut "C:/Users/a/Documents/nx_open_integrations/cpp/vms_server_plugins/opencv_object_detection_analytics_plugin/step5/build/nx_kit/unit_tests/Debug/nx_kit_ut.exe")
  set_tests_properties(nx_kit_ut PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;76;add_test;C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(nx_kit_ut "C:/Users/a/Documents/nx_open_integrations/cpp/vms_server_plugins/opencv_object_detection_analytics_plugin/step5/build/nx_kit/unit_tests/Release/nx_kit_ut.exe")
  set_tests_properties(nx_kit_ut PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;76;add_test;C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(nx_kit_ut "C:/Users/a/Documents/nx_open_integrations/cpp/vms_server_plugins/opencv_object_detection_analytics_plugin/step5/build/nx_kit/unit_tests/MinSizeRel/nx_kit_ut.exe")
  set_tests_properties(nx_kit_ut PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;76;add_test;C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(nx_kit_ut "C:/Users/a/Documents/nx_open_integrations/cpp/vms_server_plugins/opencv_object_detection_analytics_plugin/step5/build/nx_kit/unit_tests/RelWithDebInfo/nx_kit_ut.exe")
  set_tests_properties(nx_kit_ut PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;76;add_test;C:/Users/a/Documents/metadata_sdk/nx_kit/unit_tests/CMakeLists.txt;0;")
else()
  add_test(nx_kit_ut NOT_AVAILABLE)
endif()
