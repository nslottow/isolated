project(isolated)

cmake_minimum_required(VERSION 2.8)

# Include GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW tests")
set(GLFW_INSTALL OFF CACHE BOOL "Create GLFW install targets")

add_subdirectory(../extlib/glfw glfw)
include_directories(../extlib/glfw/include)

# Build the src directory
add_subdirectory(../src isolated)