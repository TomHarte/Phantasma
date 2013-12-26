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

class Object;
typedef std::map<uint16_t, std::shared_ptr<Object>> ObjectMap;

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

	private:
		uint16_t areaID;
		ObjectMap *objectsByID;
		ObjectMap *entrancesByID;

		Object *objectWithIDFromMap(ObjectMap *map, uint16_t objectID);
};

#endif /* defined(__Phantasma__Area__) */
