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
/*	static const char *names[] =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"building",
		"awning",
		"block",
		
	};*/

	struct PositionComparitor
	{
		float *position;
		
		bool operator() (Object *object1, Object *object2)
		{
//			std::cout << names[object1->getObjectID()] << " v " << names[object2->getObjectID()] << "; ";

			Vector3d objectOrigins[2]	= {	object1->getOrigin(),	object2->getOrigin()	};
			Vector3d objectSizes[2]		= {	object1->getSize(),		object2->getSize()		};

			for(int axis = 0; axis < 3; axis++)
			{
				// figure out which box is lower down this axis than the other
				bool invertResult;
				int lowerObject;
				
				if(objectOrigins[1][axis] < objectOrigins[0][axis])
				{
					invertResult = true;
					lowerObject = 1;
				}
				else
				{
					invertResult = false;
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
						bool result = (position[axis] >= endOfLowerObject) ^ invertResult;
//						std::cout << "axis: " << names[(result ? object1->getObjectID() : object2->getObjectID())] << std::endl;
						return result;
					}
				}
			}

			// if no separation was found, sort by object ID
//			std::cout << "ID: " << names[((object1->getObjectID() < object2->getObjectID()) ? object1->getObjectID() : object2->getObjectID())] << std::endl;
			return object1->getObjectID() < object2->getObjectID();
		}
	} comparitor;
	comparitor.position = playerPosition;

//	std::cout << "===" << std::endl;
	std::sort(drawableObjects.begin(), drawableObjects.end(), comparitor);

	for(std::vector<Object *>::iterator iterator = drawableObjects.begin(); iterator != drawableObjects.end(); iterator++)
	{
//		std::cout << names[(*iterator)->getObjectID()] << " ";
		(*iterator)->draw(areaBuffer);
	}

//	std::cout << std::endl;
}
