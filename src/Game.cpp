#include "Game.h"

#include "Color.h"
#include "Config.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

using namespace std;

Game::Game(int width, int height) :
	mMaxPlayers(4), mNumPlayers(2),
	mWidth(width), mHeight(height),
	mCells(width * height)
{
	// Initialize edge cells for the fillEmptyRegions algorithm
	initEdges();

	// Setup players
	for (int i = 0; i < 2; ++i) {
		mPlayers.push_back(make_shared<Player>(*this, i, i));
		mPlayers[i]->position.x = (float)(rand() % mWidth);
		mPlayers[i]->position.y = (float)(rand() % mHeight);
	}
}

void Game::initEdges() {
	for (int i = 0; i < mWidth; ++i) {
		for (int j = 0; j < mHeight; ++j) {
			auto& cell = getCellAt(i, j);
			cell.nextWallX = mWidth - i;
			cell.nextWallY = mHeight - j;
		}
	}
}

WallPtr Game::createWall(int x, int y, int playerId) {
	if (!isInBounds(x, y)) {
		return nullptr;
	}

	auto& wall = getWallAt(x, y);
	if (!wall) {
		// Update the cells to the left
		for (int i = x - 1; i >= 0; --i) {
			auto& cell = getCellAt(i, y);
			cell.nextWallX = x - i;

			if (cell.wall) {
				break;
			}
		}

		// Update the cells below
		for (int j = y - 1; j >= 0; --j) {
			auto& cell = getCellAt(x, j);
			cell.nextWallY = y - j;

			if (cell.wall) {
				break;
			}
		}

		wall = make_shared<Wall>(*this, x, y, playerId);
		return wall;
	} else if (wall->getPlayerId() == playerId) {
		// Let WallStreams create through a player's own walls
		// TODO: This logic could probably be in Player
		return wall;
	}

	return nullptr;
}

void Game::removeWall(int x, int y) {
	auto& cell = getCellAt(x, y);
	auto& wall = cell.wall;
	if (!wall) {
		return;
	}

	wall.reset();

	// Update the cells to the left
	for (int i = x - 1; i >= 0; --i) {
		auto& leftCell = getCellAt(i, y);
		leftCell.nextWallX = cell.nextWallX + x - i;

		if (leftCell.wall) {
			break;
		}
	}

	// Update the cells below
	for (int j = y - 1; j >= 0; --j) {
		auto& rightCell = getCellAt(x, j);
		rightCell.nextWallY = cell.nextWallY + y - j;

		if (rightCell.wall) {
			break;
		}
	}
}

// TODO: Make this into attack cell, and have it affect players too
bool Game::attackWall(int x, int y, char damage) {
	if (!isInBounds(x, y)) {
		return false;
	}

	auto& cell = getCellAt(x, y);
	auto& wall = cell.wall;
	if (!wall) {
		return false;
	}

	wall->takeDamage(damage);
	if (!wall->active) {
		removeWall(x, y);
	}

	return true;
}

static const struct DirectionInfo {
	int dxNext, dyNext;
	int dxWall, dyWall;
} kDirectionInfo[] = {
	{ 1, 0, 0, -1 }, // Right
	{ 0, 1, 1, 0 }, // Up
	{ -1, 0, 0, 1 }, // Left
	{ 0, -1, -1, 0 }, // Down
};

bool Game::isEdgeContiguous(int& x, int& y, int directionIndex) {
	auto& edge = kDirectionInfo[directionIndex];
	while (isInBounds(x, y) && !getWallAt(x, y)) {
		int wallX = x + edge.dxWall;
		int wallY = y + edge.dyWall;
		if (isInBounds(wallX, wallY) && !getWallAt(wallX, wallY)) {
			// This edge is not contiguous so this region is not a rectangle
			return false;
		}

		x += edge.dxNext;
		y += edge.dyNext;
	}

	x -= edge.dxNext;
	y -= edge.dyNext;
	return true;
}

bool Game::getRectangularRegion(int x, int y, int initialDirectionIndex, int& left, int& bottom, int& right, int& top) {
	// If the first cell in this region is out of bounds or a wall
	// then there can't possibly be an empty rectangle there.
	if (!isInBounds(x, y) || getWallAt(x, y)) {
		return false;
	}

	left = right = x;
	bottom = top = y;

	/*auto& initialDirection = kDirectionInfo[initialDirectionIndex];
	int checkX = x + initialDirection.dxNext;
	int checkY = y + initialDirection.dyNext;*/
	int checkX = x;
	int checkY = y;

	for (unsigned int edgeIndex = 0; edgeIndex < 4; ++edgeIndex) {
		int edgeDirectionIndex = (initialDirectionIndex + edgeIndex) % 4;
		if (!isEdgeContiguous(checkX, checkY, edgeDirectionIndex)) {
			return false;
		}

		if (checkX < left) {
			left = checkX;
		} else if (checkX > right) {
			right = checkX;
		}
		
		if (checkY < bottom) {
			bottom = checkY;
		} else if (checkY > top) {
			top = checkY;
		}
	}

	// Continue along the original edge until we arrive back at the start
	if (!isEdgeContiguous(checkX, checkY, initialDirectionIndex)) {
		return false;
	}

	return true;
}

bool Game::isRegionEmpty(int left, int bottom, int right, int top) {
	int regionWidth = right - left + 1;
	int regionHeight = top - bottom + 1;

	for (int i = left; i <= right; ++i) {
		if (getCellAt(i, bottom).nextWallY != regionHeight) {
			return false;
		}
	}

	for (int j = bottom; j <= top; ++j) {
		if (getCellAt(left, j).nextWallX != regionWidth) {
			return false;
		}
	}

	return true;
}

/**
 * Given the coordinates of a newly constructed or moved wall, check for potentially
 * created rectangles full of empty cells and fill them.
 *
 * Algorithm:
 * Each cell holds the distance to the next wall or edge of the map in the positive X/Y directions
 * When a wall is created, update the cells in the negative X/Y directions with their distance to the new wall.
 * Then, scan the edges of the potentially created rectangles in the four directions surrounding the cell.
 * If the left and bottom edges of a scanned region all report the same distance to the next wall, then the region is an empty rectangle.
 * When walls are destroyed or pushed, we need to update the cells to the left and below
 */
void Game::fillEmptyRegions(int x, int y, int playerId) {
	// Beginning from the cells adjacent to this new wall in each direction
	// attempt to make a counter-clockwise loop along the empty cells touching the walls.
	// If this is possible using only left turns, then the region is a rectangle.
	for (unsigned int checkDirectionIndex = 0; checkDirectionIndex < 4; ++checkDirectionIndex) {
		auto& checkDirection = kDirectionInfo[checkDirectionIndex];
		int startX = x + checkDirection.dxNext;
		int startY = y + checkDirection.dyNext;

		// Attempt to make a counter-clockwise loop starting and ending at (startX, startY)
		unsigned int initialDirectionIndex = (checkDirectionIndex - 1) % 4;
		int left, bottom, top, right;

		if (getRectangularRegion(startX, startY, initialDirectionIndex, left, bottom, right, top)) {
			// The region in this direction is a rectangle
			if (isRegionEmpty(left, bottom, right, top)) {
				// The region in this direction is empty, so fill it
				for (int i = left; i <= right; ++i) {
					for (int j = bottom; j <= top; ++j) {
						createWall(i, j, playerId);
					}
				}
			}
		}
	}
}

void Game::update(float dt) {
	mClock.advance(dt);

	// Update walls
	for (int i = 0; i < mWidth; ++i) {
		for (int j = 0; j < mHeight; ++j) {
			auto& cell = getCellAt(i, j);
			if (cell.wall) {
				if (cell.wall->active) {
					cell.wall->update(dt);
				} else {
					removeWall(i, j);
				}
			}
		}
	}

	// Update players
	for (auto& player : mPlayers) {
		player->update(dt);
	}

	collidePlayersWithWorld();
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
	glOrtho(-1., mWidth + 1., -1., mHeight + 1., -1.f, 1.f);
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
			auto& cell = getCellAt(i, j);
			auto& wall = cell.wall;
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

	// Render fill algorithm debug info
	//glBegin(GL_POINTS);
	//for (int i = 0; i < mWidth; ++i) {
	//	for (int j = 0; j < mHeight; ++j) {
	//		auto& cell = getCellAt(i, j);
	//		// Draw the distance to the next wall
	//		float spacing = 0.07f;
	//		for (int k = 1; k <= cell.nextWallX; ++k) {
	//			glColor3f(0.f, 1.f, 1.f);
	//			glVertex2f(i + spacing * (k + 1), j + spacing);
	//		}

	//		for (int k = 1; k <= cell.nextWallY; ++k) {
	//			glColor3f(1.f, 0.f, 1.f);
	//			glVertex2f(i + spacing, j + spacing * (k + 1));
	//		}
	//	}
	//}
	//glEnd();

	// Render players
	for (auto& player : mPlayers) {
		glColor4fv((const GLfloat*)&playerColors[player->getPlayerId()]);
		player->renderDebug();
	}
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

void Game::collidePlayerWithWall(PlayerPtr player, WallPtr wall) {
	float px1 = player->position.x;
	float px2 = px1 + player->size.x;
	float py1 = player->position.y;
	float py2 = py1 + player->size.y;

	float wx1 = wall->position.x;
	float wx2 = wx1 + wall->size.x;
	float wy1 = wall->position.y;
	float wy2 = wy1 + wall->size.y;

	// Separating axis theorem on x-axis
	float pushApartX;
	bool overlapsX = intersect(px1, px2, wx1, wx2, pushApartX);
	
	// Separating axis theorem on y-axis
	float pushApartY;
	bool overlapsY = intersect(py1, py2, wy1, wy2, pushApartY);

	// The player collides with the wall if they intersect on both the x and y axes
	if (overlapsX && overlapsY) {
		float absPushApartX = fabsf(pushApartX);
		float absPushApartY = fabsf(pushApartY);

		// Kill the player if it is more than half in the square
		float playerArea = player->size.x * player->size.y;
		if (absPushApartX * absPushApartY > playerArea * 0.5f) {
			player->die();
			// TODO: Implement a proper respawn mechanism depending on the mode
			int newX = rand() % mWidth;
			int newY = rand() % mHeight;
			removeWall(newX, newY);
			player->position.x = (float)newX;
			player->position.y = (float)newY;
			return;
		}

		// Only move the player half the distance out of the wall
		pushApartX *= 0.5f;
		pushApartY *= 0.5f;

		// Push the player away from the wall along the axis with the least overlap
		if (absPushApartX < absPushApartY) {
			player->position.x += pushApartX;
		} else if (absPushApartX > absPushApartY) {
			player->position.y += pushApartY;
		} else {
			player->position.x += pushApartX;
			player->position.y += pushApartY;
		}
	}
}

void Game::collidePlayersWithWorld() {
	for (auto& player : mPlayers) {
		if (!player->active) {
			continue;
		}

		boundEntity(static_pointer_cast<Entity>(player));

		// Check all the cells that the player could possibly be in
		int startX = (int)player->position.x;
		int startY = (int)player->position.y;
		int endX = (int)(player->position.x + player->size.x);
		int endY = (int)(player->position.y + player->size.y);

		for (int i = startX; i <= endX; ++i) {
			if (i >= mWidth) {
				break;
			}

			for (int j = startY; j <= endY; ++j) {
				if (j >= mHeight) {
					continue;
				}

				auto& wall = getWallAt(i, j);
				if (wall && wall->active) {
					collidePlayerWithWall(player, wall);
				}
			}
		}
	}
}