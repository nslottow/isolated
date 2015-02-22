#include "Game.h"

#include "Color.h"
#include "Config.h"
#include "FillRules.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

using namespace std;

Game::Game(int width, int height) :
	mMaxPlayers(4), mNumPlayers(2),
	mWidth(width), mHeight(height),
	mWalls(width * height),
	mSpatialHash(width * height),
	mNextEntityId(0)
{
	mFillRule.reset(new EmptyRectanglesFillRule(*this));
	mFillRule->onInit();

	// Setup players
	for (int i = 0; i < 2; ++i) {
		createPlayer(rand() % mWidth, rand() % mHeight);
	}
}

int Game::popNextEntityId() {
	int entityId;
	if (mFreeEntityIds.empty()) {
		entityId = mNextEntityId++;
	} else {
		entityId = mFreeEntityIds.back();
		mFreeEntityIds.pop_back();
	}

	return entityId;
}

void Game::createPlayer(int x, int y) {
	auto player = make_shared<Player>(*this, popNextEntityId(), mPlayers.size());
	mPlayers.push_back(player);
	player->position.x = (float)x;
	player->position.y = (float)y;
}

WallPtr Game::createWall(int x, int y, int playerId) {
	if (!isInBounds(x, y)) {
		return nullptr;
	}

	auto wall = getWallAt(x, y);
	if (!wall) {
		wall = make_shared<Wall>(*this, x, y, popNextEntityId(), playerId);
		setWallAt(x, y, wall);
		mFillRule->onWallCreated(x, y);
		return wall;
	} else if (wall->getPlayerId() == playerId) {
		// Let WallStreams create through a player's own walls
		// TODO: This logic could probably be in Player
		return wall;
	}

	return nullptr;
}

void Game::removeWall(int x, int y) {
	auto wall = getWallAt(x, y);
	if (!wall) {
		return;
	}

	mFreeEntityIds.push_back(wall->getEntityId());
	setWallAt(x, y, nullptr);

	mFillRule->onWallDestroyed(x, y);
}

// TODO: Make this into attack cell, and have it affect players too
bool Game::attackWall(int x, int y, char damage) {
	if (!isInBounds(x, y)) {
		return false;
	}

	auto wall = getWallAt(x, y);
	if (!wall) {
		return false;
	}

	wall->takeDamage(damage);
	if (!wall->active) {
		removeWall(x, y);
	}

	return true;
}

void Game::onWallCompleted(int x, int y) {
	mFillRule->onWallCompleted(x, y);
}

void Game::update(float dt) {
	mClock.advance(dt);

	// Update walls
	for (int i = 0; i < mWidth; ++i) {
		for (int j = 0; j < mHeight; ++j) {
			auto wall = getWallAt(i, j);
			if (wall) {
				if (wall->active) {
					wall->update(dt);
				} else {
					removeWall(i, j);
				}
			}
		}
	}

	// Update dynamic walls
	for (auto wall : mDynamicWalls) {
		wall->update(dt);
	}

	// Update players
	for (auto& player : mPlayers) {
		player->update(dt);
	}

	collideEntities();
}

void Game::renderDebug() {
	static Color playerColors[] = {
		{1.f, 0.f, 0.f, 0.5f},
		{0.f, 1.f, 0.f, 0.5f}
	};

	// Setup camera
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1., mWidth + 1., -1., mHeight + 1., -1., 1.);
	glMatrixMode(GL_MODELVIEW);

	// Render grid
	Color gridColor = gConfig["debug"].getColor("grid-color");
	glColor4fv((GLfloat*)&gridColor);
	glBegin(GL_LINES);
	for (int i = 0; i <= mWidth; ++i) {
		glVertex2i(i, 0);
		glVertex2i(i, mHeight);
	}

	for (int j = 0; j <= mHeight; ++j) {
		glVertex2i(0, j);
		glVertex2i(mWidth, j);
	}
	glEnd();

	// Render walls
	glBegin(GL_QUADS);
	for (int i = 0; i < mWidth; ++i) {
		for (int j = 0; j < mHeight; ++j) {
			auto wall = getWallAt(i, j);
			if (wall) {
				auto& pos = wall->position;
				float height = wall->getHeight() * 0.5f;
				Color baseColor = playerColors[wall->getPlayerId()];
				Color topColor = baseColor;
				float strength = (float)wall->getStrength() / Wall::sMaxStrength;
				topColor *= 0.7f * strength;
				topColor.a = 1.f;
				baseColor *= strength;
				glColor4fv((GLfloat*)&baseColor);
				glVertex2f(pos.x + 0.f, pos.y + 0.f);
				glVertex2f(pos.x + 1.f, pos.y + 0.f);
				glVertex2f(pos.x + 1.f, pos.y + 1.f);
				glVertex2f(pos.x + 0.f, pos.y + 1.f);

				glColor4fv((GLfloat*)&topColor);
				glVertex2f(pos.x + 0.f, pos.y + height);
				glVertex2f(pos.x + 1.f, pos.y + height);
				glVertex2f(pos.x + 1.f, pos.y + 1.0f);
				glVertex2f(pos.x + 0.f, pos.y + 1.0f);
			}
		}
	}
	glEnd();

	// Render players
	for (auto& player : mPlayers) {
		glColor4fv((const GLfloat*)&playerColors[player->getPlayerId()]);
		player->renderDebug();
	}

	glDisable(GL_BLEND);
}

void Game::boundEntity(EntityPtr entity) {
	auto& pos = entity->position;
	auto& size = entity->size;

	if (pos.x < 0.f) {
		pos.x = 0.f;
	} else if (pos.x + size.x >= mWidth) {
		pos.x = mWidth - size.x;
	}

	if (pos.y < 0.f) {
		pos.y = 0.f;
	} else if (pos.y + size.y >= mHeight) {
		pos.y = mHeight - size.y;
	}
}

/**
 * Intersect the 1D line segment given by (a1, a2) with (b1, b2)
 * @return true if a intersects b
 * @param pushApart if a intersects b, holds the push apart vector to move a out of the collision
 */
static bool intersect(float a1, float a2, float b1, float b2, float& pushApart) {
	float pushApartDir = -1.f;

	if (a1 > b1) {
		swap(a1, b1);
		swap(a2, b2);
		pushApartDir = 1.f;
	}

	if (a1 <= b1 && a2 >= b1) {
		pushApart = (a2 - b1) * pushApartDir;
		return true;
	}

	return false;
}

void Game::updateSpatialHash() {
	// mSpatialHash[x + y * mWidth]
	for (auto& cell : mSpatialHash) {
		cell.clear();
	}

	for (auto player : mPlayers) {
		addEntityToSpatialHash(player);
	}

	for (auto wall : mDynamicWalls) {
		addEntityToSpatialHash(wall);
	}
}

void Game::addEntityToSpatialHash(EntityPtr entity) {
	// Add the entity to all the cells that it overlaps
	int startX = (int)entity->position.x;
	int startY = (int)entity->position.y;
	int endX = (int)(entity->position.x + entity->size.x * 0.999f);
	int endY = (int)(entity->position.y + entity->size.y * 0.999f);

	for (int i = startX; i <= endX; ++i) {
		if (i >= mWidth) {
			break;
		}

		for (int j = startY; j <= endY; ++j) {
			if (j >= mHeight) {
				continue;
			}

			mSpatialHash[i + j * mWidth].push_back(entity);
		}
	}
}

void Game::collideEntities() {
	// Collide dynamic entities against each other using the spatial hash
	updateSpatialHash();

	for (auto& cell : mSpatialHash) {
		for (size_t i = 0; i < cell.size(); ++i) {
			for (size_t j = i + 1; j < cell.size(); ++j) {
				auto a = cell[i];
				auto b = cell[j];
				if (a != b) {
					collideEntities(a, b);
				}
			}
		}
	}

	// Collide dynamic entities with the world
	for (auto player : mPlayers) {
		collideEntityWithWorld(player);
	}

	for (auto wall : mDynamicWalls) {
		collideEntityWithWorld(wall);
	}

	// TODO: For any collisions that have not been persisted, they have been exited

	mPreviousCollisions = mCollisions;
	mCollisions.clear();

}

void Game::collideEntityWithWorld(EntityPtr entity) {
	boundEntity(entity);

	int startX = (int)entity->position.x;
	int startY = (int)entity->position.y;
	int endX = (int)(entity->position.x + entity->size.x * 0.999f);
	int endY = (int)(entity->position.y + entity->size.y * 0.999f);

	for (int i = startX; i <= endX; ++i) {
		if (i >= mWidth) {
			break;
		}

		for (int j = startY; j <= endY; ++j) {
			if (j >= mHeight) {
				continue;
			}

			auto wall = getWallAt(i, j);
			if (wall) {
				collideEntities(entity, wall);
			}
		}
	}
}

void Game::collideEntities(EntityPtr a, EntityPtr b) {
	float ax1 = a->position.x;
	float ax2 = ax1 + a->size.x;
	float ay1 = a->position.y;
	float ay2 = ay1 + a->size.y;

	float bx1 = b->position.x;
	float bx2 = bx1 + b->size.x;
	float by1 = b->position.y;
	float by2 = by1 + b->size.y;

	// Separating axis theorem on x-axis
	float pushApartX;
	bool overlapsX = intersect(ax1, ax2, bx1, bx2, pushApartX);

	// Separating axis theorem on y-axis
	float pushApartY;
	bool overlapsY = intersect(ay1, ay2, by1, by2, pushApartY);

	// The entities are colliding if they intersect on both the x and y axes
	if (overlapsX && overlapsY) {
		Vec2 overlap(pushApartX, pushApartY);

		float absPushApartX = fabsf(pushApartX);
		float absPushApartY = fabsf(pushApartY);

		if (a->dynamic && b->dynamic) {
			pushApartX *= 0.5f;
			pushApartY *= 0.5f;
		}

		// Push a half the distance away from b other along the axis with the least overlap
		// Note: this assumes both are rectangles
		if (a->dynamic) {
			if (absPushApartX < absPushApartY) {
				a->position.x += pushApartX * 0.5f;
			} else if (absPushApartX > absPushApartY) {
				a->position.y += pushApartY * 0.5f;
			} else {
				a->position.x += pushApartX * 0.5f;
				a->position.y += pushApartY * 0.5f;
			}
		}

		// Push b half the distance away from a other along the axis with the least overlap
		// Note: this assumes both are rectangles
		if (b->dynamic) {
			if (absPushApartX < absPushApartY) {
				b->position.x -= pushApartX * 0.5f;
			} else if (absPushApartX > absPushApartY) {
				b->position.y -= pushApartY * 0.5f;
			} else {
				b->position.x -= pushApartX * 0.5f;
				b->position.y -= pushApartY * 0.5f;
			}
		}

		// Check if this collision already existed
		auto collision = Collision(a, b);

		auto it = mPreviousCollisions.find(collision);
		if (it == mPreviousCollisions.end()) {
			a->onCollisionEnter(b, overlap);
			b->onCollisionEnter(a, -overlap);
		} else {
			a->onCollisionPersist(b, overlap);
			b->onCollisionPersist(a, -overlap);
		}

		mCollisions.insert(collision);
	}
}
