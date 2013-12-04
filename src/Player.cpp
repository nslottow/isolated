#include "Player.h"

#include "Config.h"
#include "Game.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

using namespace std;

float Player::sBuildAdvanceTime = 0.3f;

Player::Player(Game& game, int entityId, int playerId) :
	Entity(entityId, ENTITY_PLAYER),
	mGame(game),
	mState(PLAYER_NORMAL),
	mPlayerId(playerId),
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
		if (gInput.isActive(mPlayerId, INPUT_UP)) {
			movementDir.y += 1.f;
			mFacing = DIR_UP;
		}
		if (gInput.isActive(mPlayerId, INPUT_DOWN)) {
			movementDir.y -= 1.f;
			mFacing = DIR_DOWN;
		}
		if (gInput.isActive(mPlayerId, INPUT_LEFT)) {
			movementDir.x -= 1.f;
			mFacing = DIR_LEFT;
		}
		if (gInput.isActive(mPlayerId, INPUT_RIGHT)) {
			movementDir.x += 1.f;
			mFacing = DIR_RIGHT;
		}

		// TODO: Strafing state or different strafing speed
		position += movementDir * speed * dt;

		// Handle creating walls
		if (gInput.justActivated(mPlayerId, INPUT_WALL)) {
			beginBuilding();
		}

		// Handle attacking walls
		if (gInput.justActivated(mPlayerId, INPUT_MELEE)) {
			int selectionX, selectionY; // TODO: these should probably be member variables
			getSelection(selectionX, selectionY);
			mGame.attackWall(selectionX, selectionY, mMeleeStrength);
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
	glBegin(GL_TRIANGLES);
	glVertex2f(position.x + 0.f, position.y);
	glVertex2f(position.x + size.x, position.y);
	glVertex2f(position.x + size.x / 2.f, position.y + size.y);
	glEnd();

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