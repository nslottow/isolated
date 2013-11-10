#ifndef PLAYER_H
#define PLAYER_H

#include "Wall.h"
#include <memory>

class Game;

class Player : public Entity {
private:
	enum State {
		PLAYER_NORMAL,
		PLAYER_BUILDING,
		PLAYER_BUILDING_ADVANCING,
		PLAYER_PUSHING,
		PLAYER_STUNNED
	} mState;

	Game& mGame;
	int mPlayerId;
	int mStock;
	char mMeleeStrength;
	Direction mFacing;

	WallPtr mWall;
	int mWallStreamX, mWallStreamY;
	Timer mBuildAdvanceTimer;

public:
	static float sBuildAdvanceTime;

	int numWalls;
	char wallStrength;
	char pushStrength;
	float speed;

	Player(Game& game, int entityId, int playerId);

	int getPlayerId() const { return mPlayerId; }
	int getStock() const { return mStock; }
	void getSelection(int& selectionX, int& selectionY) const;

private:
	void advanceBuilding();
	void tryCreateWall();
	void beginBuilding();

public:
	void die(); // Destroy the WallStream and deactivate this Player

	void update(float dt); // Handle input and move!

	void renderDebug() const;
};

typedef std::shared_ptr<Player> PlayerPtr;

#endif
