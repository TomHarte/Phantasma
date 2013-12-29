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
//	int c = 0;
	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
	{
		if(iterator->second->isDrawable())
		{
//			c++;
//			if(
//				(c == 5) ||
//				(c == 6) ||
//				(c == 7)
//			)
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

int compareObjects(Object *object1, Object *object2, float *position)
{
	Vector3d objectOrigins[2]	= {	object1->getOrigin(),	object2->getOrigin()	};
	Vector3d objectSizes[2]		= {	object1->getSize(),		object2->getSize()		};

	int results[3] = {0, 0, 0};

	for(int axis = 0; axis < 3; axis++)
	{
		// figure out which box is lower down this axis than the other
		int invertResult;
		int lowerObject;

		if(objectOrigins[1][axis] < objectOrigins[0][axis])
		{
			invertResult = -1;
			lowerObject = 1;
		}
		else
		{
			invertResult = 1;
			lowerObject = 0;
		}

		// is there a separating plane between these two at all?
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
	}

	// if no opinion was expressed then the two are coplanar, so compare by ID
	if(!results[0] && !results[1] && !results[2])
		return (object1->getObjectID() < object2->getObjectID()) ? -1 : 1;

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
