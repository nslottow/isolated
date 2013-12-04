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

DebugMenuItemInt::DebugMenuItemInt(const char* category, const char* key, int min, int max) :
	DebugMenuItem(category, key),
	mMin(min),
	mMax(max)
{
	mValue = gConfig[mCategory].getInt(mKey, mMin);
}

void DebugMenuItemInt::update(float dt) {
	static stringstream converter;

	int oldValue = mValue;

	if (gInput.isActive(INPUT_LEFT)) {
		--mValue;
		if (mValue < mMin) {
			mValue = mMin;
		}
	}
	
	if (gInput.isActive(INPUT_RIGHT)) {
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

DebugMenuItemString::DebugMenuItemString(const char* category, const char* key, initializer_list<const char*> options) :
	DebugMenuItem(category, key),
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
	if (gInput.justActivated(INPUT_UP)) {
		mSelectionIndex = (mSelectionIndex - 1) % mItems.size();
	}

	if (gInput.justActivated(INPUT_DOWN)) {
		mSelectionIndex = (mSelectionIndex + 1) % mItems.size();
	}

	assert(!mItems.empty());
	mItems[mSelectionIndex]->update(dt);
}

void DebugMenu::render() {
	glPushMatrix();
	for (size_t i = 0; i < mItems.size(); ++i) {
		auto item = mItems[i];

		if (mSelectionIndex == i) {
			glColor4fv((GLfloat*)&mSelectedColor);
		} else {
			glColor4fv((GLfloat*)&mUnselectedColor);
		}

		item->render();
		glTranslatef(0.f, -DebugFont::kGlyphHeight - 1.f, 0.f);
	}
	glPopMatrix();
}

SceneGameSetup::SceneGameSetup(int screenWidth, int screenHeight) :
	mScreenWidth(screenWidth), mScreenHeight(screenHeight),
	mFontScale(3.f)
{
	mMenu.addItem(new DebugMenuItemString("defaults", "mode", { "territory", "stock", "smash" }));
	mMenu.addItem(new DebugMenuItemString("defaults", "fill-rule", { "empty-rectangles", "empty-regions", "3-surround" }));
	mMenu.addItem(new DebugMenuItemInt("defaults", "grid-size", 2, 100));
	mMenu.addItem(new DebugMenuItemInt("defaults", "time-limit", 0, 600));
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