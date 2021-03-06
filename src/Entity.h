#ifndef ENTITY_H
#define ENTITY_H

#include "Vector.h"
#include <memory>

enum Direction {
	DIR_RIGHT = 0,
	DIR_UP = 1,
	DIR_LEFT = 2,
	DIR_DOWN = 3,
};

enum EntityType {
	ENTITY_PLAYER,
	ENTITY_WALL
};

class Entity {
private:
	int mEntityId;
	EntityType mEntityType;

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
	Entity(int id, EntityType type) :
		mEntityId(id), mEntityType(type), active(true) {}

	int getEntityId() const { return mEntityId; }
	EntityType getType() const { return mEntityType; }
};

typedef std::shared_ptr<Entity> EntityPtr;

#endif
