#include "Wall.h"

#include "FillRules.h"
#include "Game.h"
#include <GLFW/glfw3.h>
#include <iostream>

// TODO: These will probably end up being members of Wall once powerups are added
float Wall::sRiseTime = 0.7f;
float Wall::sFallTime = 0.7f;
float Wall::sMovementSpeed = 15.f;
int Wall::sMaxStrength = 3;

Wall::Wall(Game& game, int x, int y, int entityId, int playerId, bool projectile) :
	Entity(entityId, ENTITY_WALL), // TODO: Give walls an entityId
	mState(WALL_RISING),
	mGame(game),
	mPlayerId(playerId),
	mStrength(sMaxStrength),
	mBuildTimer(game.getClock(), sRiseTime)
{
	dynamic = false;
	mIsProjectile = projectile;
	position = Vec2((float)x, (float)y);
	size = Vec2(1.f, 1.f);
}

float Wall::getHeight() const {
	if (mState == WALL_RISING) {
		return mBuildTimer.getQuadraticInterpolator();
	} else if (mState == WALL_FALLING) {
		return 1.f - mBuildTimer.getQuadraticInterpolator();
	} else {
		return 1.f;
	}
}

void Wall::beginRising() {
	if (mState == WALL_FALLING) {
		// Resume rising from our current height
		mBuildTimer.setDuration(sRiseTime - mBuildTimer.getElapsedTime());
		mState = WALL_RISING;
	}
}

void Wall::beginFalling() {
	if (mState == WALL_RISING) {
		// Begin falling from our current height as long as we are not static
		mBuildTimer.setDuration(sFallTime - mBuildTimer.getElapsedTime());
		mState = WALL_FALLING;
	}
}

void Wall::takeDamage(int damage) {
	mStrength -= damage;
	if (mStrength <= 0) {
		die();
	}
}

void Wall::beginMoveTo(int x, int y) {
	if (active && x >= 0 && x < mGame.getWidth() && y >= 0 && y < mGame.getHeight()) {
		mMoveTargetX = x;
		mMoveTargetY = y;
		mMovementDir = Vec2((float)x - position.x, (float)y - position.y);
		mMovementDir.normalize();
		mState = WALL_MOVING;
		assert(mGame.getWallAt((int)position.x, (int)position.y).get() == this);
		mGame.onWallBeginMove((int)position.x, (int)position.y);
	}
}

void Wall::die() {
	active = false;
	mStrength = 0;
	// TODO: notify our WallStream that it should stop
}

void Wall::onCollisionEnter(EntityPtr other, Vec2 overlap) {
	if (other->getType() == ENTITY_WALL && mState == WALL_MOVING) {
		auto otherDir = other->position - position;
		otherDir.normalize();
		if (otherDir.x * mMovementDir.x + otherDir.y * mMovementDir.y > 0.8f) {
			position = Vec2(roundf(position.x), roundf(position.y));
			mState = WALL_STATIC;
			mGame.onWallFinishMove(getEntityId());
		}
	}
}

void Wall::update(float dt) {
	if (mState == WALL_RISING) {
		if (mBuildTimer.isExpired()) {
			mState = WALL_STATIC;
			assert(mGame.getWallAt((int)position.x, (int)position.y).get() == this);
			mGame.onWallCompleted((int)position.x, (int)position.y);
		}
	} else if (mState == WALL_FALLING) {
		if (mBuildTimer.isExpired()) {
			die();
		}
	} else if (mState == WALL_MOVING) {
		Vec2 moveTarget((float)mMoveTargetX, (float)mMoveTargetY);
		float distance = (moveTarget - position).length();
		if (distance < 0.05f) {
			position = moveTarget;
			mState = WALL_STATIC;
			mGame.onWallFinishMove(getEntityId());
		} else {
			position += mMovementDir * sMovementSpeed * dt;
		}
	}

	// TODO: This Wall should be taken off the update list when it transitions to state WALL_STATIC
}
