cmake_minimum_required(VERSION 3.11)

file(GLOB physics_sources "*.cpp" "${INCLUDE_DIR}/physics/*.h")

add_library(physics_lib STATIC ${physics_sources})
target_include_directories(physics_lib PUBLIC CONAN_PKG::boost ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(physics_lib PUBLIC CONAN_PKG::boost Threads::Threads)