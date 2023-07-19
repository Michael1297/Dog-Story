cmake_minimum_required(VERSION 3.11)

file(GLOB sources "*.cpp" "${INCLUDE_DIR}/*.h")

add_executable(game_server ${sources})
target_include_directories(game_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(game_server PRIVATE model_lib server_lib)