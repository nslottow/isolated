#ifndef SCENE_GAME_SETUP_H
#define SCENE_GAME_SETUP_H

#include "Color.h"
#include "Scene.h"
#include <initializer_list>
#include <string>
#include <vector>

// HACK: This whole scene is a hack

class DebugMenuItem {
protected:
	const char* mCategory;
	const char* mKey;
	bool mActive;

public:
	DebugMenuItem(const char* category, const char* key, bool active) :
		mCategory(category), mKey(key), mActive(active) {}

	bool isActive() const { return mActive; }
	void setActive(bool active) { mActive = active; }

	virtual void update(float dt) {}

	virtual void render();
};

class DebugMenuItemInt : public DebugMenuItem {
private:
	int mValue;
	int mMin;
	int mMax;
	float mSpeedupTime;
	float mTimeUntilSpeedup;
	bool mKeyDown;

public:
	DebugMenuItemInt(const char* category, const char* key, int min, int max, bool active = true);

	void update(float dt) override;
};

class DebugMenuItemString : public DebugMenuItem {
private:
	std::vector<std::string> mOptions;
	int mSelectionIndex;

public:
	DebugMenuItemString(const char* category, const char* key, std::initializer_list<const char*> options, bool active = true);

	void update(float dt) override;
};

class DebugMenu {
private:
	int mScreenHeight;
	int mSelectionIndex;
	std::vector<DebugMenuItem*> mItems;
	Color mUnselectedColor;
	Color mSelectedColor;
	Color mDisabledColor;

public:
	DebugMenu() :
		mSelectionIndex(0),
		mUnselectedColor(.5f, .5f, .5f, .5f),
		mSelectedColor(.2f, .8f, 1.f, 1.f),
		mDisabledColor(.2f, .2f, .2f, .5f) {}

	~DebugMenu();

	void addItem(DebugMenuItem* item) { mItems.push_back(item); }

	void update(float dt);

	void render();
};

class SceneGameSetup : public Scene {
private:
	int mScreenWidth;
	int mScreenHeight;
	float mFontScale;

	DebugMenu mMenu;
	// State
	// Which item am I on
	// What choice is that item on

public:
	SceneGameSetup(int screenWidth, int screenHeight);

	void onActivate() override;

	void onKeyEvent(int key, int action, int mods) override;

	void update(float dt) override;

	void render() override;
};

#endif
