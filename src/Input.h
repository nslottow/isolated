#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <string>
#include <vector>

enum InputType {
	INPUT_TYPE_KEY,
	INPUT_TYPE_JOY_BUTTON,
	INPUT_TYPE_JOY_AXIS
};

enum PlayerInput {
	INPUT_UP = 0,
	INPUT_DOWN = 1,
	INPUT_LEFT = 2,
	INPUT_RIGHT = 3,
	INPUT_WALL = 4,
	INPUT_MELEE = 5,
	INPUT_COUNT = 6
};

struct PlayerInputState {
	bool current[INPUT_COUNT];
	bool previous[INPUT_COUNT];
	float holdTime[INPUT_COUNT];

	PlayerInputState();
};

struct InputAxis {
	int playerId;
	PlayerInput input;
};

inline bool operator<(const InputAxis a, const InputAxis b) {
	return a.playerId < b.playerId || a.input < b.input;
}

struct JoyButton {
	int joyId;
	int code;
};

inline bool operator<(const JoyButton a, const JoyButton b) {
	return a.joyId < b.joyId || a.code < b.code;
}

struct JoyAxis {
	int joyId;
	int axis;
	char sign;
};

inline bool operator<(const JoyAxis& a, const JoyAxis& b) {
	return a.joyId < b.joyId || a.axis < b.axis || a.sign < b.sign;
}

struct GLFWwindow;
class ConfigSection;

class Input {
private:
	std::map<std::string, int> mNameKeyMap;
	std::map<InputAxis, InputType> mInputTypeMap;
	std::map<int, InputAxis> mKeyMap;
	std::map<JoyButton, InputAxis> mJoyButtonMap;
	std::map<JoyAxis, InputAxis> mJoyAxisMap;
	std::vector<PlayerInputState> mInputStates;

public:
	static const int kMaxLocalPlayers = 4;

	Input();

	bool isActive(int playerId, PlayerInput input, float& holdTime) const;
	bool isActive(int playerId, PlayerInput input) const;
	bool isActive(PlayerInput input) const;
	bool justActivated(int playerId, PlayerInput input) const;
	bool justActivated(PlayerInput input) const;
	bool justDeactivated(int playerId, PlayerInput input) const;
	bool justDeactivated(PlayerInput input) const;

	void update(GLFWwindow* window, double dt);

	void clearMappings();
	void addKeyMapping(int code, int playerId, PlayerInput input);
	void addJoyButtonMapping(int joyId, int code, int playerId, PlayerInput input);
	void addJoyAxisMapping(int joyId, int axis, char sign, int playerId, PlayerInput input);
	void removeMapping(int playerId, PlayerInput input);
	void loadMappingFromConfig();

private:
	void loadInputFromConfig(ConfigSection& config, int playerId, const char* inputName, PlayerInput input);
};

extern Input gInput;

#endif
