cmake_minimum_required(VERSION 3.11)

file(GLOB_RECURSE model_sources "*.cpp" "${INCLUDE_DIR}/model/*.h")

add_library(model_lib STATIC ${model_sources})
target_include_directories(model_lib PUBLIC CONAN_PKG::boost ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(model_lib PUBLIC CONAN_PKG::boost Threads::Threads physics_lib database_lib)