#ifndef FILL_RULES_H
#define FILL_RULES_H

#include "Game.h"

class IFillRule {
public:
	virtual void onInit() = 0;

	virtual void onWallCreated(int x, int y) = 0;

	virtual void onWallCompleted(int x, int y) = 0;

	virtual void onWallDestroyed(int x, int y) = 0;

	virtual void onWallMoved(int fromX, int fromY, int toX, int toY) = 0;
};

class EmptyRectanglesFillRule : public IFillRule {
private:
	struct Cell {
		int nextWallX; // Distance to the next wall in the positive x direction
		int nextWallY; // Distance to the next wall in the positive y direction
	};

	std::vector<Cell> mCells;
	Game& mGame;

	Cell& getCellAt(int x, int y) {
		assert(mGame.isInBounds(x, y));
		return mCells[x + y * mGame.getWidth()];
	}

	bool isEdgeContiguous(int& x, int& y, int directionIndex);
	bool getRectangularRegion(int x, int y, int initialDirectionIndex, int& left, int& bottom, int& right, int& top);
	bool isRegionEmpty(int left, int bottom, int right, int top);
	void fillEmptyRegions(int x, int y, int playerId);

public:
	EmptyRectanglesFillRule(Game& game) : mGame(game) {}

	void onInit() override;

	void onWallCreated(int x, int y) override;

	void onWallCompleted(int x, int y) override;

	void onWallDestroyed(int x, int y) override;

	void onWallMoved(int fromX, int fromY, int toX, int toY) override {}
};

#endif
