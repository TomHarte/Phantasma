//
//  Area.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Area.h"
#include "Object.h"
#include <list>

Object *Area::objectWithIDFromMap(ObjectMap *map, uint16_t objectID)
{
	if(!map) return nullptr;
	if(!map->count(objectID)) return nullptr;
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
		{
			drawableObjects.push_back(iterator->second);
		}
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

static int compareObjects(Object *object1, Object *object2, float *position)
{
	Vector3d objectOrigins[2]	= {	object1->getOrigin(),	object2->getOrigin()	};
	Vector3d objectSizes[2]		= {	object1->getSize(),		object2->getSize()		};

	int results[3] = {0, 0, 0};
	int separations = 0;

	for(int axis = 0; axis < 3; axis++)
	{
		// figure out which box is lower down this axis than the other
		int invertResult;
		int lowerObject;

		if((objectOrigins[1][axis] + objectSizes[1][axis]) <= objectOrigins[0][axis])
		{
			invertResult = -1;
			lowerObject = 1;
		}
		else
		{
			invertResult = 1;
			lowerObject = 0;
		}

		// is there a separating plane between these two?
		uint16_t endOfLowerObject = objectOrigins[lowerObject][axis] + objectSizes[lowerObject][axis];
		if(endOfLowerObject <= objectOrigins[lowerObject ^ 1][axis])
		{
			// if so, is the player positioned within the gap? We don't have
			// an opinion on draw order if that's the case
			if((position[axis] < endOfLowerObject) || (position[axis] >  objectOrigins[lowerObject ^ 1][axis]))
			{
				int result = (position[axis] >= endOfLowerObject) ? 1 : -1;
				result *= invertResult;
				results[axis] = result;
			}
		}
		else
			separations |= (1 << axis);
	}

	// if no opinion was expressed then the two are coplanar, so compare by ID
	if(separations == 0x7)
	{
		std::cout << "x: (" << objectOrigins[0][0] << " " << objectOrigins[0][0]+objectSizes[0][0] << ") (" << objectOrigins[1][0] << " " << objectOrigins[1][0]+objectSizes[1][0] << ")" << std::endl;
		std::cout << "y: (" << objectOrigins[0][1] << " " << objectOrigins[0][1]+objectSizes[0][1] << ") (" << objectOrigins[1][1] << " " << objectOrigins[1][1]+objectSizes[1][1] << ")" << std::endl;
		std::cout << "z: (" << objectOrigins[0][2] << " " << objectOrigins[0][2]+objectSizes[0][2] << ") (" << objectOrigins[1][2] << " " << objectOrigins[1][2]+objectSizes[1][2] << ")" << std::endl;
		std::cout << std::endl;
		return (object1->getObjectID() < object2->getObjectID()) ? -1 : 1;
	}

	// otherwise we need all axes with opinions to match
	int result = 0;
	for(int c = 0; c < 3; c++)
	{
		if(results[c])
		{
			if(!result)
			{
				result = results[c];
			}
			else
			{
				if(results[c] != result)
					return 0;
			}
		}
	}

	return result;
}

void Area::draw(float *playerPosition)
{
	/*
		Here's a relic for you: we're going to use painter's algorithm
		(ie, back-to-front drawing, no depth testing, overdraw to obscure)
		to draw the scene. Why? Because:

			(1)	the original engine appears to use the painter's algorith;
				I've concluded this by observing that it cannot deal with
				mutual overlap;

			(2)	because the original engine uses painter's algorithm and
				because of the geometry involved it's standard practive to
				use coplanar geometry for doors and adornments;

			(3)	depth buffers deal very poorly with coplanar geometry,
				with even use of polygon offsets being difficult to scale
				to the arbitrary case; and

			(4)	with the tiny amount of geometry involved and absolutely
				trivial single-coloured surfaces involved, the cost of
				overdraw is not a concern.

	*/
	std::list <Object *> orderedObjects;

	for(std::vector<Object *>::iterator incomingIterator = drawableObjects.begin(); incomingIterator != drawableObjects.end(); incomingIterator++)
	{
		bool didInsert = false;

		for(std::list<Object *>::iterator existingIterator = orderedObjects.begin(); existingIterator != orderedObjects.end(); existingIterator++)
		{
			int comparison = compareObjects(*incomingIterator, *existingIterator, playerPosition);

			if(comparison > 0)
			{
				didInsert = true;
				orderedObjects.insert(existingIterator, *incomingIterator);
				break;
			}
		}

		if(!didInsert)
			orderedObjects.push_back(*incomingIterator);
	}

	for(std::list<Object *>::iterator iterator = orderedObjects.begin(); iterator != orderedObjects.end(); iterator++)
	{
		(*iterator)->draw(areaBuffer);
	}
}
