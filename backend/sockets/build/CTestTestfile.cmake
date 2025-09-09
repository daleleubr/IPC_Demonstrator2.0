# CMake generated Testfile for 
# Source directory: C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets
# Build directory: C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(test_common "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/build/Debug/test_common.exe")
  set_tests_properties(test_common PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;12;add_test;C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(test_common "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/build/Release/test_common.exe")
  set_tests_properties(test_common PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;12;add_test;C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(test_common "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/build/MinSizeRel/test_common.exe")
  set_tests_properties(test_common PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;12;add_test;C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(test_common "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/build/RelWithDebInfo/test_common.exe")
  set_tests_properties(test_common PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;12;add_test;C:/Users/sanch/source/repos/IPC_Demonstrator2.0/backend/sockets/CMakeLists.txt;0;")
else()
  add_test(test_common NOT_AVAILABLE)
endif()
