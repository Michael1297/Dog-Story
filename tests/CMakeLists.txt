cmake_minimum_required(VERSION 3.11)

file(GLOB tests_sources "*.cpp")

add_executable(game_server_tests ${tests_sources})
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 model_lib physics_lib)