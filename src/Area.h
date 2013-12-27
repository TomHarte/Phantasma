//
//  Area.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Area__
#define __Phantasma__Area__

#include <iostream>
#include <map>
#include "Object.h"
#include "VertexBuffer.h"

typedef std::map<uint16_t, Object *> ObjectMap;

class Area
{
	public:
		Area(
			uint16_t areaID,
			ObjectMap *objectsByID,
			ObjectMap *entrancesByID);
		virtual ~Area();

		Object *objectWithID(uint16_t objectID);
		Object *entranceWithID(uint16_t objectID);
		uint16_t getAreaID();

		void setupOpenGL();
		void draw(float *playerPosition);

	private:
		uint16_t areaID;
		ObjectMap *objectsByID;
		ObjectMap *entrancesByID;
		std::vector<Object *> drawableObjects;

		Object *objectWithIDFromMap(ObjectMap *map, uint16_t objectID);

		VertexBuffer *areaBuffer;
};

#endif /* defined(__Phantasma__Area__) */
