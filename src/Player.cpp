#include "Player.h"

#include "Config.h"
#include "Game.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

using namespace std;

float Player::sRespawnTime = 1.5f;
float Player::sBuildAdvanceTime = 0.3f;
float Player::sProjectileAdvanceTime = 0.05f;
float Player::sAttackTapTime = 0.15f;
float Player::sPushStartTime = 0.4f;

Player::Player(Game& game, int entityId, int playerId) :
	Entity(entityId, ENTITY_PLAYER),
	mGame(game),
	mState(PLAYER_NORMAL),
	mPlayerId(playerId),
	mMovementHoldTime(0.f),
	mFacing(DIR_UP),
	mMeleeStrength(1),
	speed(5.f),
	mRespawnTimer(game.getClock(), sRespawnTime),
	mBuildAdvanceTimer(game.getClock(), sBuildAdvanceTime),
	mProjectileAdvanceTimer(game.getClock(), sProjectileAdvanceTime)
{
	mStock = gConfig["defaults"].getInt("stock", 10);
	size = Vec2(1.f, 1.f);
}

void Player::getSelection(int& selectionX, int& selectionY) const {
	selectionX = (int)(position.x + size.x / 2.f);
	selectionY = (int)(position.y + size.y / 2.f);
	switch (mFacing) {
	case DIR_UP:
		++selectionY;
		break;
	case DIR_DOWN:
		--selectionY;
		break;
	case DIR_LEFT:
		--selectionX;
		break;
	case DIR_RIGHT:
		++selectionX;
		break;
	}
}

void Player::tryCreateWall(State desiredState) {
	mWall = mGame.createWall(mWallStreamX, mWallStreamY, mPlayerId, desiredState == PLAYER_BUILDING_PROJECTILE);
	if (mWall) {
		mState = desiredState;
		mWall->beginRising();
	} else {
		// We were unable to build, so we can't continue building
		mState = PLAYER_NORMAL;
		mWall.reset();
	}
}

void Player::advanceWallStream() {
	// Continue building in the direciton the player is facing
	switch (mFacing) {
	case DIR_UP:
		++mWallStreamY;
		break;
	case DIR_DOWN:
		--mWallStreamY;
		break;
	case DIR_LEFT:
		--mWallStreamX;
		break;
	case DIR_RIGHT:
		++mWallStreamX;
		break;
	}

	if (mWallStreamX < 0) {
		mWallStreamX = 0;
	} else if (mWallStreamX >= mGame.getWidth()) {
		mWallStreamX = mGame.getWidth() - 1;
	}

	if (mWallStreamY < 0) {
		mWallStreamY = 0;
	} else if (mWallStreamY >= mGame.getHeight()) {
		mWallStreamY = mGame.getHeight() - 1;
	}
}

void Player::die() {
	mState = PLAYER_NORMAL;
	mWall.reset();
	active = false;
	mRespawnTimer.reset();

	// TODO: Implement a proper respawn mechanism depending on the mode
	int newX = rand() % mGame.getWidth();
	int newY = rand() % mGame.getHeight();
	position.x = (float)newX;
	position.y = (float)newY;

	--mStock;
	cout << "Player " << mPlayerId << " died, " << mStock << " lives left" << endl;
	if (mStock == 0) {
		cout << "Player " << mPlayerId << " lost" << endl;
	}
}

void Player::onCollisionEnter(EntityPtr other, Vec2 pushApart) {
	if (other->getType() == ENTITY_WALL) {
		float absPushApartX = fabsf(pushApart.x);
		float absPushApartY = fabsf(pushApart.y);

		// Kill the player if it is more than half in the square
		float playerArea = size.x * size.y;
		if (absPushApartX * absPushApartY > playerArea * 0.5f) {
			die();
		}
	}
}

void Player::onCollisionPersist(EntityPtr other, Vec2 pushApart) {
	if (other->getType() == ENTITY_WALL) {
		float absPushApartX = fabsf(pushApart.x);
		float absPushApartY = fabsf(pushApart.y);

		// Kill the player if it is more than half in the square
		float playerArea = size.x * size.y;
		if (absPushApartX * absPushApartY > playerArea * 0.5f) {
			die();
		}
	}
}

void Player::update(float dt) {
	if (!active) {
		if (mRespawnTimer.isExpired()) {
			active = true;
			mGame.removeWall((int)position.x, (int)position.y);
		}
		return;
	}

	if (mState == PLAYER_NORMAL) {
		// Handle movement
		Vec2 movementDir;
		float holdTime = -1.f;
		float minHoldTime = 2e32f;

		if (gInput.isActive(mPlayerId, INPUT_UP, holdTime)) {
			movementDir.y += 1.f;
			if (holdTime < minHoldTime) {
				mFacing = DIR_UP;
				minHoldTime = holdTime;
			}
		}
		if (gInput.isActive(mPlayerId, INPUT_DOWN, holdTime)) {
			movementDir.y -= 1.f;
			if (holdTime < minHoldTime) {
				mFacing = DIR_DOWN;
				minHoldTime = holdTime;
			}
		}
		if (gInput.isActive(mPlayerId, INPUT_LEFT, holdTime)) {
			movementDir.x -= 1.f;
			if (holdTime < minHoldTime) {
				mFacing = DIR_LEFT;
				minHoldTime = holdTime;
			}
		}
		if (gInput.isActive(mPlayerId, INPUT_RIGHT, holdTime)) {
			movementDir.x += 1.f;
			if (holdTime < minHoldTime) {
				mFacing = DIR_RIGHT;
				minHoldTime = holdTime;
			}
		}

		if (minHoldTime < 2e32f) {
			mMovementHoldTime = minHoldTime;
		}

		// TODO: Strafing state or different strafing speed
		position += movementDir * speed * dt;

		// Update selection
		getSelection(mSelectionX, mSelectionY);

		// Handle creating walls
		if (gInput.justActivated(mPlayerId, INPUT_WALL)) {
			// Try to begin building outward from the player's selection
			getSelection(mWallStreamX, mWallStreamY);
			tryCreateWall(PLAYER_BUILDING);
		} else if (gInput.justActivated(mPlayerId, INPUT_MELEE)) {
			// Try to begin building a projectile at the selection
			getSelection(mWallStreamX, mWallStreamY);
			tryCreateWall(PLAYER_BUILDING_PROJECTILE);
		}

		// Handle attacking walls
		PlayerInput attackInput;
		switch (mFacing) {
		case DIR_UP:
			attackInput = INPUT_UP;
			break;
		case DIR_DOWN:
			attackInput = INPUT_DOWN;
			break;
		case DIR_LEFT:
			attackInput = INPUT_LEFT;
			break;
		case DIR_RIGHT:
			attackInput = INPUT_RIGHT;
			break;
		}

		if (gInput.justDeactivated(attackInput) && mMovementHoldTime < sAttackTapTime) {
			mGame.attackWall(mSelectionX, mSelectionY, mMeleeStrength);
		}
	}
	
	// Handle updating the building state
	if (mState == PLAYER_BUILDING || mState == PLAYER_BUILDING_ADVANCING) {
		if (gInput.justDeactivated(mPlayerId, INPUT_WALL)) {
			mState = PLAYER_NORMAL;
			mWall->beginFalling();
			mWall.reset();
		}
	}

	if (mState == PLAYER_BUILDING) {
		if (mWall->isComplete()) {
			mBuildAdvanceTimer.reset();
			mState = PLAYER_BUILDING_ADVANCING;
		}
	}

	if (mState == PLAYER_BUILDING_ADVANCING) {
		if (mBuildAdvanceTimer.isExpired()) {
			advanceWallStream();
			tryCreateWall(PLAYER_BUILDING);
		}
	}

	// Handle updating the projectile state
	if (mState == PLAYER_BUILDING_PROJECTILE) {
		if (mWall->isComplete()) {
			mProjectileAdvanceTimer.reset();
			mState = PLAYER_CHARGING_PROJECTILE;
		}

		if (gInput.justDeactivated(mPlayerId, INPUT_MELEE)) {
			mState = PLAYER_NORMAL;
			mWall->beginFalling();
			mWall.reset();
		} 
	}

	if (mState == PLAYER_CHARGING_PROJECTILE) {
		if (mProjectileAdvanceTimer.isExpired()) {
			mProjectileAdvanceTimer.reset();
			advanceWallStream();
		}

		if (gInput.justDeactivated(mPlayerId, INPUT_MELEE)) {
			mState = PLAYER_NORMAL;
			// Release the projectile
			// TODO: pull this out into a function so it happens if we die while charging too
			releaseProjectileWall();
		}
	}
}

void Player::releaseProjectileWall() {
	int targetX = mSelectionX;
	int targetY = mSelectionY;

	switch (mFacing) {
	case DIR_UP:
		targetY = mGame.getHeight() - 1;
		break;
	case DIR_DOWN:
		targetY = 0;
		break;
	case DIR_LEFT:
		targetX = 0;
		break;
	case DIR_RIGHT:
		targetX = mGame.getWidth() - 1;
		break;
	}

	mWall->beginMoveTo(targetX, targetY);
	mWall.reset();
}

static void renderCircle(float x, float y, float r) {
	glBegin(GL_LINE_STRIP);
	const int steps = 32;
	const float angleStep = 6.28318531f / steps;
	for (int i = 0; i < steps; i++) {
		glVertex2f(x + r * cos(i * angleStep), y + r * sin(i * angleStep));
	}

	glVertex2f(x + r, y);
	glEnd();
}

void Player::renderDebug() const {
	// Render player stock at the bottom of the screen
	glPointSize(5.f);
	glBegin(GL_POINTS);
	float stockBarOffset = mPlayerId * 4.f;
	for (int i = 0; i < mStock; ++i) {
		glVertex2f(i * 0.4f + stockBarOffset, -0.5f);
	}
	glEnd();

	if (!active) {
		renderCircle(position.x + size.x / 2.f, position.y + size.y / 2.f, 0.5f / mRespawnTimer.getInterpolator());
		return;
	}

	// Render the player body
	float angle = 0.f;
	switch (mFacing) {
	case DIR_UP:
		break;
	case DIR_LEFT:
		angle = 90.f;
		break;
	case DIR_DOWN:
		angle = 180.f;
		break;
	case DIR_RIGHT:
		angle = 270.f;
		break;
	}

	glPushMatrix();
	glTranslatef(position.x + size.x / 2.f, position.y + size.y / 2.f, 0.f);
	glRotatef(angle, 0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLES);
	glVertex2f(-size.x / 2.f, -size.y / 2.f); // Bottom left
	glVertex2f(size.x / 2.f, -size.y / 2.f); // Bottom right
	glVertex2f(0.f, size.y / 2.f); // Top
	glEnd();
	glPopMatrix();

	// Render the selection
	if (mState == PLAYER_NORMAL) {
		int selectionX, selectionY;
		getSelection(selectionX, selectionY);

		glColor4f(0.3f, 0.3f, 0.5f, 0.3f);
		glBegin(GL_QUADS);
		glVertex2i(selectionX, selectionY);
		glVertex2i(selectionX + 1, selectionY);
		glVertex2i(selectionX + 1, selectionY + 1);
		glVertex2i(selectionX, selectionY + 1);
		glEnd();
	}

	// Render the currently building indicator
	if (mState == PLAYER_BUILDING || mState == PLAYER_BUILDING_ADVANCING) {
		glColor4f(1.f, 1.f, 1.f, 0.1f);
		glBegin(GL_LINE_STRIP);
		glVertex2i(mWallStreamX, mWallStreamY);
		glVertex2i(mWallStreamX + 1, mWallStreamY);
		glVertex2i(mWallStreamX + 1, mWallStreamY + 1);
		glVertex2i(mWallStreamX, mWallStreamY + 1);
		glVertex2i(mWallStreamX, mWallStreamY);
		glEnd();
	}
}