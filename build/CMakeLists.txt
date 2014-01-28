project(isolated)

cmake_minimum_required(VERSION 2.8)

if (MSVC)
	set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "Suppress regeneration of VS project files")
endif()

function(create_vcxproj_userfile TARGETNAME)
  if (MSVC)
    configure_file(
          ${CMAKE_SOURCE_DIR}/VisualStudioUserFile.vcxproj.user.in
          ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
          @ONLY
        )
  endif()
endfunction(create_vcxproj_userfile)

# Include GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW tests")
set(GLFW_INSTALL OFF CACHE BOOL "Create GLFW install targets")

add_subdirectory(../extlib/glfw glfw)
include_directories(../extlib/glfw/include)

# Include SOIL
add_subdirectory(../extlib/soil soil)
include_directories(../extlib)

# Include boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Build the src directory
add_subdirectory(../src isolated)