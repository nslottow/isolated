#ifndef SCENE_GAME_SETUP_H
#define SCENE_GAME_SETUP_H

#include "Scene.h"

// Number of players
// Grid width, height
// Mode
// Time limit
// Stock
// Fill rule

class DebugMenuItem {
public:
	// Types: Button, Left/Right Selector, Slider, Time
	// MenuItem(const char** choices);

	virtual void onSelect();
	
	virtual void onDeselect();

	virtual void update(float dt);

	void render();
};

class DebugMenu {
private:
	void update(float dt);

	void render();
};

class SceneGameSetup : public Scene {
private:
	int mScreenWidth;
	int mScreenHeight;
	float mFontScale;

public:
	SceneGameSetup(int screenWidth, int screenHeight) :
		mScreenWidth(screenWidth), mScreenHeight(screenHeight), mFontScale(3.f) {}

	void onActivate() override;

	void onKeyEvent(int key, int action, int mods) override;

	void update(float dt) override;

	void render() override;
};

#endif
