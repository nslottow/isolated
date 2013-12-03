#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <stack>

class Scene {
public:
	virtual void onActivate() {}

	virtual void onDeactivate() {}

	virtual void update(float dt) {}

	virtual void render() {}
};

typedef std::shared_ptr<Scene> ScenePtr;

class SceneStack {
private:
	std::stack<ScenePtr> mScenes;

public:
	bool isEmpty() const { return mScenes.empty(); }

	void push(ScenePtr scene);

	void pop();

	void update(float dt);

	void render();
};

extern std::shared_ptr<SceneStack> gScenes;

#endif