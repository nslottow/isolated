#include "Scene.h"

#include <cassert>

std::shared_ptr<SceneStack> gScenes;

void SceneStack::push(ScenePtr scene) {
	if (!mScenes.empty()) {
		mScenes.top()->onDeactivate();
	}

	scene->onActivate();
	mScenes.push(scene);
}

void SceneStack::pop() {
	assert(!mScenes.empty());
	mScenes.top()->onDeactivate();
	mScenes.pop();
}

void SceneStack::update(float dt) {
	if (!mScenes.empty()) {
		mScenes.top()->update(dt);
	}
}

void SceneStack::render() {
	if (!mScenes.empty()) {
		mScenes.top()->render();
	}
}
