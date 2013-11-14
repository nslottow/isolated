#ifndef GAME_GRID_H
#define GAME_GRID_H

#include "Player.h"
#include "Time.h"
#include "Wall.h"
#include <cassert>
#include <istream>
#include <memory>
#include <vector>

// Could make this more event driven...
// The important things happen on events, and movement is just a smooth interface to trigger discrete events
// This game may be too simple to warrant an event system where objects register for events they care about

class FillRule {
public:
	virtual void onInit() = 0 {} // Initialize per-cell data

	virtual void onWallCreated(int x, int y) = 0 {}

	virtual void onWallCompleted(int x, int y) = 0 {}

	virtual void onWallDestroyed(int x, int y) = 0 {}

	virtual void onWallMoved(int fromX, int fromY, int toX, int toY) = 0 {}

private:
	// Somehow give this access to the cells, isInBounds, getCellAt, getWallAt, createWall, destroyWall
	// Maybe just have this allocate cells, and have the game allocate wall ptrs & entities from pools
	// Maybe have FillRule be a friend class of Game?
	// Maybe have game make isInBounds, getCellAt, and getWallAt public
};

class VictoryRule {
public:
	// These are represented by a victory rule:
	// Win Conditions: Fill, Stock, Kills, Time

	// These probably go in Game:
	// Per Game Settings: Wall rise time, fall time, extend time, strength, pushable, push time, player speed, powerups, teams, fillrule

	// These probably go in Player:
	// Stats Per Player: Walls created, walls destroyed, walls lost, walls moved, kills, deaths, time alive
	//   probably want to store these outside the victory rule and in the player

	// These can be calculated post-game:
	// Stats Per Team

	// onInit
	// onEntityCreated
	// onEntityDestroyed
};

// Rules are just event handlers that check for certain things on events
// A game could have any number of VictoryRules
// A game could have any number of FillRules too... but should probably only have one, otherwise it'd be too confusing

// Other events
// onCollisionEnter(Entity) - fired by Game, handled by Entities
// onCollisionStay(Entity) - fired by Game, handled by Entities - useful for pushing walls, only want to push walls we're colliding with for a while
// onCollisionExit(Entity) - fired by Game, handled by Entities
// onEntityCreated - fired by Game - Game handles creation of entities
// onEntityDestroyed - fired by Game - Game handles destruction of entities, wall has to notify game that it wants to be destroyed by setting active = false
// onTakeDamage - fired by Wall, handled by WallSprite
// onWallCompleted - fired by Wall, handled by WallSprite, FillRule/Game
// onWallMoved - fired by Wall, handled by Game
// onPlayerDeath - fired by Player, handled by Game

// Walls moving shouldn't screw up the state of the game grid... so only the game should be able to initiate it-
//   or, the game should account for walls being moved when it updates them

class Game {
private:
	struct Cell {
		int nextWallX; // Distance to the next wall in the positive x direction
		int nextWallY; // Distance to the next wall in the positive y direction
		WallPtr wall;
		WallPtr incomingWall;
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
