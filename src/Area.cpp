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
	vertexBuffer = nullptr;
	drawElementsBuffer = nullptr;

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
	delete vertexBuffer;
	vertexBuffer = GeometricObject::newVertexBuffer();

	delete drawElementsBuffer;
	drawElementsBuffer = GeometricObject::newDrawElementsBuffer();

	for(ObjectMap::iterator iterator = objectsByID->begin(); iterator != objectsByID->end(); iterator++)
		iterator->second->setupOpenGL(vertexBuffer, drawElementsBuffer);
}

void Area::draw(bool allowPolygonOffset)
{
	bool polygonOffsetIsEnabled = false;
	if(allowPolygonOffset)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	for(std::vector<Object *>::iterator iterator = drawableObjects.begin(); iterator != drawableObjects.end(); iterator++)
	{
		if(allowPolygonOffset && (*iterator)->isPlanar() != polygonOffsetIsEnabled)
		{
			polygonOffsetIsEnabled ^= true;
			if(polygonOffsetIsEnabled)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
			}
			else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
		}

		(*iterator)->draw(vertexBuffer, drawElementsBuffer);
	}
}
