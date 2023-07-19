cmake_minimum_required(VERSION 3.11)

file(GLOB database_sources "*.cpp" "${INCLUDE_DIR}/database/*.h")

add_library(database_lib STATIC ${database_sources})
target_include_directories(database_lib PUBLIC CONAN_PKG::boost ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(database_lib PUBLIC CONAN_PKG::boost CONAN_PKG::libpq CONAN_PKG::libpqxx Threads::Threads)