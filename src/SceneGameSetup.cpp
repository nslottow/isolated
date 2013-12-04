#include "SceneGameSetup.h"

#include "Config.h"
#include "DebugFont.h"
#include "Input.h"
#include "SceneLocalGame.h"

using namespace std;

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
	// TODO: Update selection based on input

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

	glColor4f(1.f, 1.f, 1.f, 1.f);
	glPushMatrix();
	glTranslatef(mScreenWidth / mFontScale / 2.f, 0.f, 0.f);
	gDebugFont->renderString("Press enter or wall to start", true);
	glPopMatrix();
}