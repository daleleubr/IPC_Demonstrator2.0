cmake_minimum_required(VERSION 3.12)
project(SocketTests)

enable_testing()


add_executable(test_common test_common.cpp)
target_link_libraries(test_common ws2_32)

add_test(NAME test_common COMMAND test_common)