//
//  Area.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Area.h"
#include "Object.h"

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16_t objectID)
{
	if(!map) return NULL;
	if(!map->count(objectID)) return NULL;
	return (*map)[objectID];
}

Object *Area::objectWithID(uint16_t objectID)
{
	return objectWithIDFromMap(objectsByID, objectID);
}

Object *Area::entranceWithID(uint16_t objectID)
{
	return objectWithIDFromMap(entrancesByID, objectID);
}

uint16_t Area::getAreaID()
{
	return areaID;
}

Area::Area(
	uint16_t _areaID,
	ObjectMap *_objectsByID,
	ObjectMap *_entrancesByID)
{
	areaID = _areaID;
	objectsByID = _objectsByID;
	entrancesByID = _entrancesByID;
	areaBuffer = nullptr;

	// create a list of drawable obejcts only
	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
	{
		if(iterator->second->isDrawable())
			drawableObjects.push_back(iterator->second);
	}
}

Area::~Area()
{
	for(ObjectMap::iterator iterator = entrancesByID->begin(); iterator != entrancesByID->end(); iterator++)
		delete iterator->second;

	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
		delete iterator->second;

	delete entrancesByID;
	delete objectsByID;
}

void Area::setupOpenGL()
{
	delete areaBuffer;
	areaBuffer = GeometricObject::newVertexBuffer();

	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
		iterator->second->setupOpenGL(areaBuffer);
}

void Area::draw(float *playerPosition)
{
	struct PositionComparitor
	{
		float *position;
		
		bool operator() (Object *object1, Object *object2)
		{
			Vector3d object1Origin = object1->getOrigin();
			Vector3d object1Size = object1->getSize();

			Vector3d object2Origin = object2->getOrigin();
			Vector3d object2Size = object2->getSize();

			// maybe x is a separating plane?
			if((object1Origin.x + object1Size.x) < object2Origin.x)
			{
				return position[0] > object1Origin.x + object1Size.x;
			}

			if((object2Origin.x + object2Size.x) < object1Origin.x)
			{
				return position[0] < object2Origin.x + object2Size.x;
			}

			// y then?
			if((object1Origin.y + object1Size.y) < object2Origin.y)
			{
				return position[1] > object1Origin.y + object1Size.y;
			}

			if((object2Origin.y + object2Size.y) < object1Origin.y)
			{
				return position[1] < object2Origin.y + object2Size.y;
			}
			
			// z?
			if((object1Origin.z + object1Size.z) < object2Origin.z)
			{
				return position[2] > object1Origin.z + object1Size.z;
			}

			if((object2Origin.z + object2Size.z) < object1Origin.z)
			{
				return position[2] < object2Origin.z + object2Size.z;
			}

			return object1->getObjectID() < object2->getObjectID();
		}
	} comparitor;
	comparitor.position = playerPosition;

	std::sort(drawableObjects.begin(), drawableObjects.end(), comparitor);

	for(std::vector<Object *>::iterator iterator = drawableObjects.begin(); iterator != drawableObjects.end(); iterator++)
		(*iterator)->draw(areaBuffer);
}
