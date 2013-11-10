#include "Input.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

Input gInput;

PlayerInputState::PlayerInputState() {
	for (int i = 0; i < INPUT_COUNT; ++i) {
		current[i] = false;
		previous[i] = false;
	}
}

Input::Input() :
	mInputStates(kMaxLocalPlayers)
{
	// Setup the default key mappings
	addKeyMapping(GLFW_KEY_UP, 0, INPUT_UP);
	addKeyMapping(GLFW_KEY_DOWN, 0, INPUT_DOWN);
	addKeyMapping(GLFW_KEY_LEFT, 0, INPUT_LEFT);
	addKeyMapping(GLFW_KEY_RIGHT, 0, INPUT_RIGHT);
	addKeyMapping(GLFW_KEY_COMMA, 0, INPUT_WALL);
	addKeyMapping(GLFW_KEY_PERIOD, 0, INPUT_MELEE);

	addKeyMapping(GLFW_KEY_W, 1, INPUT_UP);
	addKeyMapping(GLFW_KEY_S, 1, INPUT_DOWN);
	addKeyMapping(GLFW_KEY_A, 1, INPUT_LEFT);
	addKeyMapping(GLFW_KEY_D, 1, INPUT_RIGHT);
	addKeyMapping(GLFW_KEY_1, 1, INPUT_WALL);
	addKeyMapping(GLFW_KEY_2, 1, INPUT_MELEE);
}

bool Input::isActive(int playerId, PlayerInput input) const {
	assert(input < INPUT_COUNT);
	return mInputStates[playerId].current[input];
}

bool Input::justActivated(int playerId, PlayerInput input) const {
	assert(input < INPUT_COUNT);
	auto& state = mInputStates[playerId];
	return state.current[input] && !state.previous[input];
}

bool Input::justDeactivated(int playerId, PlayerInput input) const {
	assert(input < INPUT_COUNT);
	auto& state = mInputStates[playerId];
	return !state.current[input] && state.previous[input];
}

void Input::onKeyEvent(int key, int scancode, int action, int mods) {
	// Ignore key repeat events
	if (action == GLFW_REPEAT || action == GLFW_RELEASE) {
		return;
	}

	auto it = mKeyMap.find(key);
	if (it != mKeyMap.end()) {
		auto& axis = it->second;
		mInputStates[axis.playerId].current[axis.input] = action == GLFW_PRESS;
	}
}

void Input::update(GLFWwindow* window) {
	for (auto& state : mInputStates) {
		for (int i = 0; i < INPUT_COUNT; ++i) {
			state.previous[i] = state.current[i];
			state.current[i] = false;
		}
	}

	for (auto& p : mKeyMap) {
		auto key = p.first;
		auto& axis = p.second;
		mInputStates[axis.playerId].current[axis.input] = glfwGetKey(window, key) == GLFW_PRESS;
	}
}

void Input::clearMappings() {
	mInputTypeMap.clear();
	mKeyMap.clear();
	mJoyButtonMap.clear();
	mJoyAxisMap.clear();
}

void Input::addKeyMapping(int code, int playerId, PlayerInput input) {
	removeMapping(playerId, input);
	mKeyMap[code] = {playerId, input};
}

void Input::addJoyButtonMapping(int joyId, int code, int playerId, PlayerInput input) {
	removeMapping(playerId, input);
	mJoyButtonMap[{joyId, code}] = {playerId, input};
}

void Input::addJoyAxisMapping(int joyId, int axis, char sign, int playerId, PlayerInput input) {
	removeMapping(playerId, input);
	mJoyAxisMap[{joyId, axis, sign}] = {playerId, input};
}

template <typename KeyT>
void eraseAxis(map<KeyT, InputAxis>& mapping, InputAxis& axis) {
	auto it = find_if(mapping.begin(), mapping.end(), [&axis](const pair<KeyT, InputAxis>& item) {
		auto& a = item.second;
		return a.playerId == axis.playerId && a.input == axis.input;
	});

	if (it != mapping.end()) {
		mapping.erase(it);
	}
}

void Input::removeMapping(int playerId, PlayerInput input) {
	// Do nothing if the input was not already mapped
	InputAxis axis = {playerId, input};
	auto typeIter = mInputTypeMap.find(axis);
	if (typeIter == mInputTypeMap.end()) {
		return;
	}

	// Remove the mapping from the appropriate map
	InputType oldType = typeIter->second;
	switch (oldType) {
	case INPUT_TYPE_KEY:
		eraseAxis(mKeyMap, axis);
		break;
	case INPUT_TYPE_JOY_BUTTON:
		eraseAxis(mJoyButtonMap, axis);
		break;
	case INPUT_TYPE_JOY_AXIS:
		eraseAxis(mJoyAxisMap, axis);
		break;
	}

	// Remove the mapping from the type map
	mInputTypeMap.erase(typeIter);
}

void Input::loadMappingFromConfig() {
	// TODO: Enable loading input mappings from config
}