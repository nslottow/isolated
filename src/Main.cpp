#include "Config.h"
#include "DebugFont.h"
#include "DebugConsole.h"
#include "Game.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <cstdlib>
#include <iostream>
#include <memory>

#ifdef WIN32
extern "C" void __stdcall Sleep(int millis);
#endif

using namespace std;

GLuint gDebugFontTexture;

void run(GLFWwindow* window) {
	// Load settings from config
	auto& config = gConfig["defaults"];
	Wall::sRiseTime = config.getFloat("wall-rise-time", Wall::sRiseTime);
	Wall::sFallTime = config.getFloat("wall-fall-time", Wall::sFallTime);
	Wall::sMaxStrength = config.getInt("wall-strength", Wall::sMaxStrength);
	Player::sBuildAdvanceTime = config.getFloat("build-advance-time", Player::sBuildAdvanceTime);
	unique_ptr<Game> game(new Game(config.getInt("grid-width", 10), config.getInt("grid-height", 10)));

	double fixedTimeStep = 1 / 60.f;
	double accumulatedTime = 0.;
	double lastFrameTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		accumulatedTime += glfwGetTime() - lastFrameTime;
		lastFrameTime = glfwGetTime();
		while (accumulatedTime >= fixedTimeStep) {
			accumulatedTime -= fixedTimeStep;
			if (!gConsole->isOpen()) {
				gInput.update(window);
			}
			game->update((float)fixedTimeStep);
		}

		glClear(GL_COLOR_BUFFER_BIT);
		game->renderDebug();
		gConsole->render();
		glfwSwapBuffers(window);
		glfwPollEvents();
#ifdef WIN32
		Sleep(1);
#endif
	}
}

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	gConfig.addFile("data/settings.ini"); // TODO: make cwd data directory
	auto& config = gConfig["application"];
	int width = config.getInt("width", 800);
	int height = config.getInt("height", 600);
	bool fullscreen = config.getBool("fullscreen");

	cout << "Display mode: "
		<< (fullscreen ? "Fullscreen " : "Windowed ")
		<< width << 'x' << height << endl;

	window = glfwCreateWindow(width, height, "Isolated", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, onKeyEvent);
	glfwSetCharCallback(window, onCharacterEvent);
	gDebugFont.reset(new DebugFont());
	int fontScale = gConfig["debug"].getInt("console-font-scale", 2);
	gConsole.reset(new DebugConsole(width, height / 2, width, height, fontScale));

	gConfig.addFile("data/controls.ini");
	gInput.loadMappingFromConfig();

	run(window);

	gDebugFont.reset();
	gConsole.reset();
	glfwTerminate();
	return 0;
}