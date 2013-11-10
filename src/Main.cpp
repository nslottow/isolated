#include "Config.h"
#include "Game.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <memory>

#ifdef WIN32
extern "C" void __stdcall Sleep(int millis);
#endif

using namespace std;

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
			gInput.update(window);
			game->update((float)fixedTimeStep);
		}

		glClear(GL_COLOR_BUFFER_BIT);
		game->renderDebug();
		glfwSwapBuffers(window);
		glfwPollEvents();
#ifdef WIN32
		Sleep(1);
#endif
	}
}

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
	gInput.onKeyEvent(key, scancode, action, mods);
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
	// glfwSetKeyCallback(window, onKeyEvent);

	run(window);
	glfwTerminate();
	return 0;
}