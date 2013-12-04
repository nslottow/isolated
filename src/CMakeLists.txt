add_executable(isolated
	Color.h Color.cpp
	Config.h Config.cpp
	DebugConsole.h DebugConsole.cpp
	DebugFont.h DebugFont.cpp
	Entity.h
	FillRules.h FillRules.cpp
	Game.h Game.cpp
	Input.h Input.cpp
	Main.cpp
	Player.h Player.cpp
	Scene.h Scene.cpp
	SceneGameSetup.h SceneGameSetup.cpp
	SceneLocalGame.h SceneLocalGame.cpp
	Time.h
	Wall.h Wall.cpp)

target_link_libraries(isolated glfw ${GLFW_LIBRARIES} soil)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	add_definitions(-Wall -std=c++11)
else (MSVC)
	add_definitions(/W3)
	set_target_properties(isolated PROPERTIES
		LINK_FLAGS      "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\""
	)
endif()

set(DEBUG_WORKING_DIR ${CMAKE_SOURCE_DIR}/..)

set_target_properties(isolated PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY         ${DEBUG_WORKING_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${DEBUG_WORKING_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${DEBUG_WORKING_DIR}
)


create_vcxproj_userfile(isolated)