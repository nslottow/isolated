#include "SceneLocalGame.h"

#include <GLFW/glfw3.h>
#include "Config.h"
#include "Game.h"
#include "Wall.h"
#include "Player.h"

void SceneLocalGame::onActivate() {
	// Load settings from config
	auto& config = gConfig["defaults"];
	Wall::sRiseTime = config.getFloat("wall-rise-time", Wall::sRiseTime);
	Wall::sFallTime = config.getFloat("wall-fall-time", Wall::sFallTime);
	Wall::sMaxStrength = config.getInt("wall-strength", Wall::sMaxStrength);
	Player::sBuildAdvanceTime = config.getFloat("build-advance-time", Player::sBuildAdvanceTime);
	mGame.reset(new Game(config.getInt("grid-width", 10), config.getInt("grid-height", 10)));
}

void SceneLocalGame::onDeactivate() {

}

void SceneLocalGame::onKeyEvent(int key, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) {
		gScenes->pop();
	}
}

void SceneLocalGame::update(float dt) {
	mGame->update(dt);
}

void SceneLocalGame::render() {
	mGame->renderDebug();
}