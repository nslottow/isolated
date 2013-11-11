if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	add_definitions(-Wall -std=c++11)
endif()

add_executable(isolated
	Color.h Color.cpp
	Config.h Config.cpp
	Game.h Game.cpp
	Input.h Input.cpp
	Main.cpp
	Player.h Player.cpp
	Time.h
	Wall.h Wall.cpp)

target_link_libraries(isolated glfw ${GLFW_LIBRARIES})

set_target_properties(isolated PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_SOURCE_DIR}/..
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/..
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/..
)
