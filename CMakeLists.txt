cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# Автоматическая установка conan
if(WIN32)
	execute_process(
			COMMAND conan install "${CMAKE_SOURCE_DIR}" --build missing -s build_type=${CMAKE_BUILD_TYPE}
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC AND MSVC_TOOLSET_VERSION EQUAL 143)
	set(CONAN_DISABLE_CHECK_COMPILER true)	# conan не поддерживает VS 2022
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

#add_compile_definitions(DISABLE_LOGGER)		# выключить логирование во время отладки если оно мешается

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src/physics) 	#physics lib
add_subdirectory(src/model) 	#model lib
add_subdirectory(src/server) 	#server lib
add_subdirectory(src/database) 	#database lib

add_subdirectory(src)	#game server

add_subdirectory(tests)	#tests