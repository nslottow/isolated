#ifndef ENTITY_H
#define ENTITY_H

#include "Vector.h"
#include <memory>

enum Direction {
	DIR_UP = 0,
	DIR_DOWN = 1,
	DIR_LEFT = 2,
	DIR_RIGHT = 3
};

class Entity {
private:
	int mEntityId;

public:
	bool active;
	bool collidable;
	bool trigger;
	Vec2 position;
	Vec2 size;

private:
	Entity(const Entity&) = delete;
	Entity(Entity&&) = delete;

public:
	Entity(int id) :
		mEntityId(id), active(true) {}
};

typedef std::shared_ptr<Entity> EntityPtr;

#endif
