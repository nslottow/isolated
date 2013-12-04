#ifndef SCENE_LOCAL_GAME_H
#define SCENE_LOCAL_GAME_H

#include "Scene.h"

class Game;

class SceneLocalGame : public Scene {
private:
	std::shared_ptr<Game> mGame;

public:
	void onActivate() override;

	void onDeactivate() override;

	void onKeyEvent(int key, int action, int mods) override;

	void update(float dt) override;

	void render() override;
};

#endif
