#include "FillRules.h"

static const struct DirectionInfo {
	int dxNext, dyNext;
	int dxWall, dyWall;
} kDirectionInfo[] = {
	{ 1, 0, 0, -1 }, // Right
	{ 0, 1, 1, 0 }, // Up
	{ -1, 0, 0, 1 }, // Left
	{ 0, -1, -1, 0 }, // Down
};

bool EmptyRectanglesFillRule::isEdgeContiguous(int& x, int& y, int directionIndex) {
	auto& edge = kDirectionInfo[directionIndex];
	while (mGame.isInBounds(x, y) && !mGame.getWallAt(x, y)) {
		int wallX = x + edge.dxWall;
		int wallY = y + edge.dyWall;
		if (mGame.isInBounds(wallX, wallY) && !mGame.getWallAt(wallX, wallY)) {
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
bool EmptyRectanglesFillRule::getRectangularRegion(int x, int y, int initialDirectionIndex, int& left, int& bottom, int& right, int& top) {
	// If the first cell in this region is out of bounds or a wall
	// then there can't possibly be an empty rectangle there.
	if (!mGame.isInBounds(x, y) || mGame.getWallAt(x, y)) {
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
bool EmptyRectanglesFillRule::isRegionEmpty(int left, int bottom, int right, int top) {
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
void EmptyRectanglesFillRule::fillEmptyRegions(int x, int y, int playerId) {
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
						mGame.createWall(i, j, playerId);
					}
				}
			}
		}
	}
}

void EmptyRectanglesFillRule::onInit() {
	auto width = mGame.getWidth();
	auto height = mGame.getHeight();

	mCells.resize(width * height);

	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			auto& cell = getCellAt(i, j);
			cell.nextWallX = width - i;
			cell.nextWallY = height - j;
		}
	}
}

void EmptyRectanglesFillRule::onWallCreated(int x, int y) {
	// Update the cells to the left
	for (int i = x - 1; i >= 0; --i) {
		auto& cell = getCellAt(i, y);
		cell.nextWallX = x - i;

		if (mGame.getWallAt(i, y)) {
			break;
		}
	}

	// Update the cells below
	for (int j = y - 1; j >= 0; --j) {
		auto& cell = getCellAt(x, j);
		cell.nextWallY = y - j;

		if (mGame.getWallAt(x, j)) {
			break;
		}
	}
}

void EmptyRectanglesFillRule::onWallCompleted(int x, int y) {
	auto playerId = mGame.getWallAt(x, y)->getPlayerId();
	fillEmptyRegions(x, y, playerId);
}

void EmptyRectanglesFillRule::onWallDestroyed(int x, int y) {
	auto& cell = getCellAt(x, y);

	// Update the cells to the left
	for (int i = x - 1; i >= 0; --i) {
		auto& leftCell = getCellAt(i, y);
		leftCell.nextWallX = cell.nextWallX + x - i;

		if (mGame.getWallAt(i, y)) {
			break;
		}
	}

	// Update the cells below
	for (int j = y - 1; j >= 0; --j) {
		auto& rightCell = getCellAt(x, j);
		rightCell.nextWallY = cell.nextWallY + y - j;

		if (mGame.getWallAt(x, j)) {
			break;
		}
	}
}