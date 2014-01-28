#include "SceneGameSetup.h"

#include <cassert>
#include <sstream>
#include "Config.h"
#include "DebugFont.h"
#include "Input.h"
#include "SceneLocalGame.h"

using namespace std;

void DebugMenuItem::render() {
	auto& config = gConfig[mCategory];
	string itemStr = (string(mKey) + ' ') + config.getString(mKey);
	gDebugFont->renderString(itemStr.c_str());
}

DebugMenuItemInt::DebugMenuItemInt(const char* category, const char* key, int min, int max, bool active) :
	DebugMenuItem(category, key, active),
	mMin(min),
	mMax(max),
	mSpeedupTime(0.5f),
	mKeyDown(false)
{
	mValue = gConfig[mCategory].getInt(mKey, mMin);
}

void DebugMenuItemInt::update(float dt) {
	static stringstream converter;

	int oldValue = mValue;

	if (gInput.justActivated(INPUT_LEFT) || gInput.justActivated(INPUT_RIGHT)) {
		mKeyDown = true;
		mTimeUntilSpeedup = mSpeedupTime;
	} else if (gInput.justDeactivated(INPUT_LEFT) || gInput.justDeactivated(INPUT_RIGHT)) {
		mKeyDown = false;
	}

	bool fastScroll = false;
	if (mKeyDown) {
		mTimeUntilSpeedup -= dt;
		if (mTimeUntilSpeedup <= 0.f) {
			mTimeUntilSpeedup = 0.f;
			fastScroll = true;
		}
	}

	if (gInput.justActivated(INPUT_LEFT) || (fastScroll && gInput.isActive(INPUT_LEFT))) {
		--mValue;
		if (mValue < mMin) {
			mValue = mMin;
		}
	}

	if (gInput.justActivated(INPUT_RIGHT) || (fastScroll && gInput.isActive(INPUT_RIGHT))) {
		++mValue;
		if (mValue > mMax) {
			mValue = mMax;
		}
	}
	
	if (mValue != oldValue) {
		converter.str("");
		converter << mValue;
		gConfig[mCategory].setProperty(mKey, converter.str().c_str());
	}
}

DebugMenuItemString::DebugMenuItemString(const char* category, const char* key, initializer_list<const char*> options, bool active) :
	DebugMenuItem(category, key, active),
	mSelectionIndex(-1)
{
	assert(options.size());

	for (auto option : options) {
		mOptions.push_back(option);
	}

	auto value = gConfig[category].getString(key, mOptions[0].c_str());
	for (size_t i = 0; i < mOptions.size(); ++i) {
		if (mOptions[i] == value) {
			mSelectionIndex = i;
		}
	}

	if (mSelectionIndex == -1) {
		mSelectionIndex = mOptions.size();
		mOptions.push_back(value);
	}
}

void DebugMenuItemString::update(float dt) {
	int oldSelectionIndex = mSelectionIndex;

	if (gInput.justActivated(INPUT_LEFT)) {
		mSelectionIndex = (mSelectionIndex - 1) % mOptions.size();
	}

	if (gInput.justActivated(INPUT_RIGHT)) {
		mSelectionIndex = (mSelectionIndex + 1) % mOptions.size();
	}

	if (mSelectionIndex != oldSelectionIndex) {
		gConfig[mCategory].setProperty(mKey, mOptions[mSelectionIndex].c_str());
	}
}

DebugMenu::~DebugMenu() {
	for (auto item : mItems) {
		delete item;
	}
}

void DebugMenu::update(float dt) {
	while (!mItems[mSelectionIndex]->isActive()) {
		mSelectionIndex = (mSelectionIndex + 1) % mItems.size();
	}

	if (gInput.justActivated(INPUT_UP)) {
		do {
			mSelectionIndex = (mSelectionIndex - 1) % mItems.size();
		} while (!mItems[mSelectionIndex]->isActive());
	}

	if (gInput.justActivated(INPUT_DOWN)) {
		do {
			mSelectionIndex = (mSelectionIndex + 1) % mItems.size();
		} while (!mItems[mSelectionIndex]->isActive());
	}

	assert(!mItems.empty());
	mItems[mSelectionIndex]->update(dt);
}

void DebugMenu::render() {
	glPushMatrix();
	for (size_t i = 0; i < mItems.size(); ++i) {
		auto item = mItems[i];

		if (!item->isActive()) {
			glColor4fv((GLfloat*)&mDisabledColor);
		} else if (mSelectionIndex == i) {
			glColor4fv((GLfloat*)&mSelectedColor);
		} else {
			glColor4fv((GLfloat*)&mUnselectedColor);
		}

		item->render();
		glTranslatef(0.f, -DebugFont::kGlyphHeight - 3.f, 0.f);
	}
	glPopMatrix();
}

SceneGameSetup::SceneGameSetup(int screenWidth, int screenHeight) :
	mScreenWidth(screenWidth), mScreenHeight(screenHeight),
	mFontScale(3.f)
{
	mMenu.addItem(new DebugMenuItemString("defaults", "mode", { "territory", "stock", "smash" }, false));
	mMenu.addItem(new DebugMenuItemString("defaults", "fill-rule", { "empty-rectangles", "empty-regions", "3-surround" }, false));
	mMenu.addItem(new DebugMenuItemString("defaults", "fill-method", { "instantaneous", "rings", "spiral", "sweep" }, false));
	mMenu.addItem(new DebugMenuItemInt("defaults", "grid-size", 2, 100));
	mMenu.addItem(new DebugMenuItemInt("defaults", "time-limit", 0, 600, false));
	mMenu.addItem(new DebugMenuItemInt("defaults", "wall-strength", 1, 10));
	mMenu.addItem(new DebugMenuItemInt("defaults", "stock", 1, 100));
}

void SceneGameSetup::onActivate() {
}

void SceneGameSetup::onKeyEvent(int key, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			gScenes->pop();
		} else if (key == GLFW_KEY_ENTER) {
			gScenes->push(make_shared<SceneLocalGame>());
		}
	}
}

void SceneGameSetup::update(float dt) {
	mMenu.update(dt);

	// Start game when wall is pressed
	if (gInput.justActivated(INPUT_WALL)) {
		gScenes->push(make_shared<SceneLocalGame>());
	}
}

void SceneGameSetup::render() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0., mScreenWidth / mFontScale, 0., mScreenHeight / mFontScale, -1., 1.);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glTranslatef((float)DebugFont::kGlyphWidth, (float)mScreenHeight / mFontScale - DebugFont::kGlyphHeight * 2, 0.f);
	mMenu.render();
	glPopMatrix();

	glColor4f(1.f, 1.f, 1.f, 1.f);
	glPushMatrix();
	glTranslatef(mScreenWidth / mFontScale / 2.f, 0.f, 0.f);
	gDebugFont->renderString("Press enter or wall to start", true);
	glPopMatrix();
}