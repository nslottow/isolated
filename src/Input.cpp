#include "Input.h"

#include "Config.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

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
	// Setup the names of mappable special keys
	mNameKeyMap["unmapped"] = GLFW_KEY_UNKNOWN;
	mNameKeyMap[""] = GLFW_KEY_UNKNOWN;
	mNameKeyMap["escape"] = GLFW_KEY_ESCAPE;
	mNameKeyMap["enter"] = GLFW_KEY_ENTER;
	mNameKeyMap["tab"] = GLFW_KEY_TAB;
	mNameKeyMap["backspace"] = GLFW_KEY_BACKSPACE;
	mNameKeyMap["insert"] = GLFW_KEY_INSERT;
	mNameKeyMap["delete"] = GLFW_KEY_DELETE;
	mNameKeyMap["right"] = GLFW_KEY_RIGHT;
	mNameKeyMap["left"] = GLFW_KEY_LEFT;
	mNameKeyMap["up"] = GLFW_KEY_UP;
	mNameKeyMap["down"] = GLFW_KEY_DOWN;
	mNameKeyMap["pageup"] = GLFW_KEY_PAGE_UP;
	mNameKeyMap["pagedown"] = GLFW_KEY_PAGE_DOWN;
	mNameKeyMap["home"] = GLFW_KEY_HOME;
	mNameKeyMap["end"] = GLFW_KEY_END;
	mNameKeyMap["capslock"] = GLFW_KEY_CAPS_LOCK;
	mNameKeyMap["scrlock"] = GLFW_KEY_SCROLL_LOCK;
	mNameKeyMap["f1"] = GLFW_KEY_F1;
	mNameKeyMap["f2"] = GLFW_KEY_F2;
	mNameKeyMap["f3"] = GLFW_KEY_F3;
	mNameKeyMap["f4"] = GLFW_KEY_F4;
	mNameKeyMap["f5"] = GLFW_KEY_F5;
	mNameKeyMap["f6"] = GLFW_KEY_F6;
	mNameKeyMap["f7"] = GLFW_KEY_F7;
	mNameKeyMap["f8"] = GLFW_KEY_F8;
	mNameKeyMap["f9"] = GLFW_KEY_F9;
	mNameKeyMap["f10"] = GLFW_KEY_F10;
	mNameKeyMap["f11"] = GLFW_KEY_F11;
	mNameKeyMap["f12"] = GLFW_KEY_F12;
	mNameKeyMap["f13"] = GLFW_KEY_F13;
	mNameKeyMap["f14"] = GLFW_KEY_F14;
	mNameKeyMap["f15"] = GLFW_KEY_F15;
	mNameKeyMap["f16"] = GLFW_KEY_F16;
	mNameKeyMap["f17"] = GLFW_KEY_F17;
	mNameKeyMap["f18"] = GLFW_KEY_F18;
	mNameKeyMap["f19"] = GLFW_KEY_F19;
	mNameKeyMap["f20"] = GLFW_KEY_F20;
	mNameKeyMap["f21"] = GLFW_KEY_F21;
	mNameKeyMap["f22"] = GLFW_KEY_F22;
	mNameKeyMap["f23"] = GLFW_KEY_F23;
	mNameKeyMap["f24"] = GLFW_KEY_F24;
	mNameKeyMap["f25"] = GLFW_KEY_F25;
	mNameKeyMap["pad0"] = GLFW_KEY_KP_0;
	mNameKeyMap["pad1"] = GLFW_KEY_KP_1;
	mNameKeyMap["pad2"] = GLFW_KEY_KP_2;
	mNameKeyMap["pad3"] = GLFW_KEY_KP_3;
	mNameKeyMap["pad4"] = GLFW_KEY_KP_4;
	mNameKeyMap["pad5"] = GLFW_KEY_KP_5;
	mNameKeyMap["pad6"] = GLFW_KEY_KP_6;
	mNameKeyMap["pad7"] = GLFW_KEY_KP_7;
	mNameKeyMap["pad8"] = GLFW_KEY_KP_8;
	mNameKeyMap["pad9"] = GLFW_KEY_KP_9;
	mNameKeyMap["pad."] = GLFW_KEY_KP_DECIMAL;
	mNameKeyMap["pad/"] = GLFW_KEY_KP_DIVIDE;
	mNameKeyMap["pad*"] = GLFW_KEY_KP_MULTIPLY;
	mNameKeyMap["pad-"] = GLFW_KEY_KP_SUBTRACT;
	mNameKeyMap["pad+"] = GLFW_KEY_KP_ADD;
	mNameKeyMap["padenter"] = GLFW_KEY_KP_ENTER;
	mNameKeyMap["lshift"] = GLFW_KEY_LEFT_SHIFT;
	mNameKeyMap["lctrl"] = GLFW_KEY_LEFT_CONTROL;
	mNameKeyMap["lalt"] = GLFW_KEY_LEFT_ALT;
	mNameKeyMap["lsuper"] = GLFW_KEY_LEFT_SUPER;
	mNameKeyMap["rshift"] = GLFW_KEY_RIGHT_SHIFT;
	mNameKeyMap["rctrl"] = GLFW_KEY_RIGHT_CONTROL;
	mNameKeyMap["ralt"] = GLFW_KEY_RIGHT_ALT;
	mNameKeyMap["rsuper"] = GLFW_KEY_RIGHT_SUPER;

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
	assert(playerId < kMaxLocalPlayers);
	return mInputStates[playerId].current[input];
}

bool Input::isActive(PlayerInput input) const {
	bool result = false;

	for (int playerId = 0; playerId < kMaxLocalPlayers; ++playerId) {
		result |= isActive(playerId, input);
	}

	return result;
}

bool Input::justActivated(int playerId, PlayerInput input) const {
	assert(input < INPUT_COUNT);
	assert(playerId < kMaxLocalPlayers);
	auto& state = mInputStates[playerId];
	return state.current[input] && !state.previous[input];
}

bool Input::justActivated(PlayerInput input) const {
	bool result = false;

	for (int playerId = 0; playerId < kMaxLocalPlayers; ++playerId) {
		result |= justActivated(playerId, input);
	}

	return result;
}

bool Input::justDeactivated(int playerId, PlayerInput input) const {
	assert(input < INPUT_COUNT);
	assert(playerId < kMaxLocalPlayers);
	auto& state = mInputStates[playerId];
	return !state.current[input] && state.previous[input];
}

bool Input::justDeactivated(PlayerInput input) const {
	bool result = false;

	for (int playerId = 0; playerId < kMaxLocalPlayers; ++playerId) {
		result |= justDeactivated(playerId, input);
	}

	return result;
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
	string sectionName = "controls-player1";
	for (int playerId = 0; playerId < kMaxLocalPlayers; ++playerId) {
		*sectionName.rbegin() = (char)(playerId + '1');
		auto& config = gConfig[sectionName];
		
		loadInputFromConfig(config, playerId, "up", INPUT_UP);
		loadInputFromConfig(config, playerId, "down", INPUT_DOWN);
		loadInputFromConfig(config, playerId, "left", INPUT_LEFT);
		loadInputFromConfig(config, playerId, "right", INPUT_RIGHT);
		loadInputFromConfig(config, playerId, "wall", INPUT_WALL);
		loadInputFromConfig(config, playerId, "melee", INPUT_MELEE);
	}
}

void Input::loadInputFromConfig(ConfigSection& config, int playerId, const char* inputName, PlayerInput input) {
	int key = GLFW_KEY_UNKNOWN;
	string keyName = config.getString(inputName);
	transform(keyName.begin(), keyName.end(), keyName.begin(), ::tolower);

	if (keyName.length() == 1) {
		key = toupper(keyName[0]);
	} else {
		key = mNameKeyMap[keyName];
	}

	if (key != GLFW_KEY_UNKNOWN) {
		addKeyMapping(key, playerId, input);
	}
}