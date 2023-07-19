cmake_minimum_required(VERSION 3.11)

file(GLOB server_sources "*.cpp" "${INCLUDE_DIR}/server/*.h")

add_library(server_lib STATIC ${server_sources})
target_include_directories(server_lib PUBLIC CONAN_PKG::boost ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(server_lib PUBLIC CONAN_PKG::boost Threads::Threads)