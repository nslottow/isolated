#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "Player.h"
#include "Time.h"
#include "Wall.h"
#include <cassert>
#include <istream>
#include <memory>
#include <vector>

class Game {
private:
	struct Cell {
		int nextWallX; // Distance to the next wall in the positive x direction
		int nextWallY; // Distance to the next wall in the positive y direction
		WallPtr wall;
	};

	int mWidth, mHeight;
	std::vector<Cell> mCells;

	int mMaxPlayers;
	int mNumPlayers;
	std::vector<PlayerPtr> mPlayers;

	Clock mClock;

private:
	void initEdges();
	bool isEdgeContiguous(int& x, int& y, int directionIndex);
	bool getRectangularRegion(int x, int y, int initialDirectionIndex, int& left, int& bottom, int& right, int& top);
	bool isRegionEmpty(int left, int bottom, int right, int top);
	// void fillEnclosedRegions(int x, int y, int playerId); // Use 

	void removeWall(int x, int y);

public:
	Game(int width, int height); // Create empty grid of the specified size
	Game(std::istream& in);	// Load a grid from the specified stream

	int getMaxPlayers() const { return mMaxPlayers; }
	int getNumPlayers() const { return mPlayers.size(); }
	PlayerPtr getPlayer(int playerId) { return mPlayers[playerId]; }

	WallPtr createWall(int x, int y, int playerId);
	bool attackWall(int x, int y, char damage); // Returns true if attack hit a Wall
	void fillEmptyRegions(int x, int y, int playerId);
	//bool pushWall(int x, int y, Direction dir, char strength);

	const Clock& getClock() const { return mClock; }

	void update(float dt);

	void renderDebug();

private:
	bool isInBounds(int x, int y) {
		return x >= 0 && x < mWidth && y >= 0 && y < mHeight;
	}

	Cell& getCellAt(int x, int y) {
		assert(isInBounds(x, y));
		return mCells[x + y * mWidth];
	}

	WallPtr& getWallAt(int x, int y) {
		return getCellAt(x, y).wall;
	}

	void boundEntity(EntityPtr entity);
	void collidePlayerWithWall(PlayerPtr player, WallPtr wall);
	void collidePlayersWithWorld();
};

#endif
