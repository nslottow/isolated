#include "Player.h"

#include "Config.h"
#include "Game.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

using namespace std;

float Player::sBuildAdvanceTime = 0.3f;
float Player::sAttackTapTime = 0.2f;
float Player::sPushStartTime = 0.4f;

Player::Player(Game& game, int entityId, int playerId) :
	Entity(entityId, ENTITY_PLAYER),
	mGame(game),
	mState(PLAYER_NORMAL),
	mPlayerId(playerId),
	mMovementHoldTime(0.f),
	mMeleeStrength(1),
	speed(5.f),
	mBuildAdvanceTimer(game.getClock(), sBuildAdvanceTime)
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

void Player::tryCreateWall() {
	mWall = mGame.createWall(mWallStreamX, mWallStreamY, mPlayerId);
	if (mWall) {
		mState = PLAYER_BUILDING;
		mWall->beginRising();
	} else {
		// We were unable to build, so we can't continue building
		mState = PLAYER_NORMAL;
		mWall.reset();
	}
}

void Player::beginBuilding() {
	assert(mState == PLAYER_NORMAL);
	getSelection(mWallStreamX, mWallStreamY);
	tryCreateWall();
}

void Player::advanceBuilding() {
	assert(mState == PLAYER_BUILDING_ADVANCING);

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

	tryCreateWall();
}

void Player::die() {
	mState = PLAYER_NORMAL;
	mWall.reset();
	--mStock;
	cout << "Player " << mPlayerId << " died, " << mStock << " lives left" << endl;
	if (mStock == 0) {
		cout << "Player " << mPlayerId << " lost" << endl;
	}
}

void Player::update(float dt) {
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
			beginBuilding();
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
			advanceBuilding();
		}
	}
}

void Player::renderDebug() const {
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

	// Render player stock at the top of the screen
	glPointSize(5.f);
	glBegin(GL_POINTS);
	float stockBarOffset = mPlayerId * 4.f;
	for (int i = 0; i < mStock; ++i) {
		glVertex2f(i * 0.4f + stockBarOffset, -0.5f);
	}
	glEnd();

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