cmake_minimum_required(VERSION 2.8.12)

project(playground)
set(CMAKE_CXX_STANDARD 11)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(src/glfw)
include_directories(${PROJECT_SOURCE_DIR})
include_directories("src")
include_directories("src/glfw/deps") # for glad
include_directories("src/glfw/include")
include_directories("src/lib")
add_executable(${PROJECT_NAME} src/main.cpp src/glfw/deps/glad.c src/sh/sh.cpp src/math/vecmath.cpp)
if (MSVC)
    add_definitions( "-D _CRT_SECURE_NO_WARNINGS" )
endif()
#if (UNIX)
#    add_definitions( "-std=c99" )
#endif()
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
