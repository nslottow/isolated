#include "Config.h"
#include "DebugFont.h"
#include "DebugConsole.h"
#include "Input.h"
#include "SceneGameSetup.h"
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>

#ifdef WIN32
extern "C" void __stdcall Sleep(int millis);
#endif

using namespace std;

void run(GLFWwindow* window) {
	double fixedTimeStep = 1 / 60.f;
	double accumulatedTime = 0.;
	double lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window) && !gScenes->isEmpty()) {
		accumulatedTime += glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();

		while (accumulatedTime >= fixedTimeStep) {
			accumulatedTime -= fixedTimeStep;

			if (!gConsole->isOpen()) {
				gInput.update(window, fixedTimeStep);
			}

			gScenes->update((float)fixedTimeStep);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		gScenes->render();
		gConsole->render();

		glfwSwapBuffers(window);
		glfwPollEvents();
#ifdef WIN32
		Sleep(1);
#endif
	}
}

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Pass the event to the current scene
	if (!gConsole->isOpen()) {
		gScenes->onKeyEvent(key, action, mods);
	}

	// Update the debug console based on the received key event
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_F1) {
			gConsole->toggleOpen();
		} else if (gConsole->isOpen()) {
			switch (key) {
			case GLFW_KEY_ENTER:
				gConsole->onCharacter('\n');
				break;
			case GLFW_KEY_BACKSPACE:
				gConsole->onCharacter(8);
				break;
			case GLFW_KEY_LEFT:
				break;
			case GLFW_KEY_RIGHT:
				break;
			case GLFW_KEY_UP:
				break;
			case GLFW_KEY_DOWN:
				break;
			case GLFW_KEY_PAGE_UP:
				break;
			case GLFW_KEY_PAGE_DOWN:
				break;
			}
		}
	}
}

void onCharacterEvent(GLFWwindow* window, unsigned int ch) {
	if (gConsole->isOpen()) {
		gConsole->onCharacter((char)ch);
	}
}

int main() {
	if (!glfwInit()) {
		return -1;
	}

	// Setup OpenGL window
	gConfig.addFile("data/settings.ini"); // TODO: make cwd data directory
	auto& config = gConfig["application"];
	int width = config.getInt("width", 600);
	int height = config.getInt("height", 600);
	bool fullscreen = config.getBool("fullscreen");

	if (fullscreen) {
		auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		width = videoMode->width;
		height = videoMode->height;

		// HACK
		stringstream converter;
		converter << width;
		config.setProperty("width", converter.str().c_str());
		converter.str("");
		converter << height;
		config.setProperty("height", converter.str().c_str());
	}

	GLFWwindow* window = glfwCreateWindow(width, height, "Isolated", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, onKeyEvent);
	glfwSetCharCallback(window, onCharacterEvent);

	// Initialize the debug font
	gDebugFont.reset(new DebugFont());
	int fontScale = gConfig["debug"].getInt("console-font-scale", 2);

	// Initialize the console
	gConsole.reset(new DebugConsole(width, height / 2, width, height, fontScale));

	// Initialize the input mappings
	gConfig.addFile("data/controls.ini");
	gInput.loadMappingFromConfig();

	// Initialize the scene stack
	gScenes.reset(new SceneStack());
	gScenes->push(make_shared<SceneGameSetup>(width, height));

	run(window);

	gScenes.reset();
	gDebugFont.reset();
	gConsole.reset();
	glfwTerminate();
	return 0;
}