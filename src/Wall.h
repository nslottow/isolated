#ifndef WALL_H
#define WALL_H

#include "Entity.h"
#include "Time.h"
#include <memory>

class Wall;
typedef std::shared_ptr<Wall> WallPtr;

class WallFiller {
public:
	// When a wall is created inside the filler's region, stop filling along that line
};

class Game;

class Wall : public Entity {
private:
	enum State {
		WALL_RISING,
		WALL_FALLING,
		WALL_CHARGING_MOVE,
		WALL_MOVING,
		WALL_STATIC
	} mState;

	Game& mGame;
	// TODO: Walls should be associated with a team in addition to a specific player, add mTeamId
	int mPlayerId;
	int mStrength;
	int mMoveTargetX, mMoveTargetY;
	Vec2 mMovementDir;
	Timer mBuildTimer;
	bool mIsProjectile;

public:
	static float sRiseTime;
	static float sFallTime;
	static float sMovementSpeed;
	static int sMaxStrength;

	Wall(Game& game, int x, int y, int entityId, int playerId, bool projectile);

	int getPlayerId() const { return mPlayerId; }
	int getStrength() const { return mStrength; }
	float getHeight() const; // Value between 0 and 1 indicating build completion
	bool isComplete() const { return mState != WALL_RISING && mState != WALL_FALLING; }

	void beginRising();
	void beginFalling();
	void takeDamage(int damage);
	void beginMoveTo(int x, int y);
	void die();


	void onCollisionEnter(EntityPtr other, Vec2 overlap) override;

	void update(float dt);
	void renderDebug();
};

#endif
