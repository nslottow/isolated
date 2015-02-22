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
		PLAYER_BUILDING_PROJECTILE,
		PLAYER_CHARGING_PROJECTILE,
		PLAYER_PUSHING,
		PLAYER_STUNNED
	} mState;

	Game& mGame;
	int mPlayerId;
	int mStock;
	char mMeleeStrength;
	int mSelectionX, mSelectionY;
	float mMovementHoldTime;
	Direction mFacing;
	Timer mRespawnTimer;
	Timer mPushTimer;
	WallPtr mPushingWall;
	int mPushTargetX, mPushTargetY;

	// Wall building state
	WallPtr mWall;
	int mWallStreamX, mWallStreamY;
	Timer mBuildAdvanceTimer;
	Timer mProjectileAdvanceTimer;

public:
	static float sRespawnTime;
	static float sBuildAdvanceTime;
	static float sProjectileAdvanceTime;
	static float sAttackTapTime;
	static float sPushStartTime;

	int numWalls;
	char wallStrength;
	char pushStrength;
	float speed;

	Player(Game& game, int entityId, int playerId);

	int getPlayerId() const { return mPlayerId; }
	int getStock() const { return mStock; }
	void getSelection(int& selectionX, int& selectionY) const;

private:
	void advanceWallStream();
	void tryCreateWall(State desiredState);
	void releaseProjectileWall();

public:
	void die(); // Destroy the WallStream and deactivate this Player

	void onWallComplete();
	void onWallDestroyed();
	void onCollisionEnter(EntityPtr other, Vec2 pushApart) override;
	void onCollisionPersist(EntityPtr other, Vec2 pushApart) override;

	void update(float dt); // Handle input and move!

	void renderDebug() const;
};

typedef std::shared_ptr<Player> PlayerPtr;

#endif
