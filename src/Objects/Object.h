//
//  Object.h
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Object__
#define __Phantasma__Object__

#include <vector>
#include "Instruction.h"

typedef struct
{
	uint16_t x, y, z;
} Vector3d;

class VertexBuffer;
class Object
{
	public:
		typedef enum
		{
			Entrance = 0,
			Cube = 1,
			Sensor = 2,
			Rectangle = 3,

			EastPyramid = 4,
			WestPyramid = 5,
			UpPyramid = 6,
			DownPyramid = 7,
			NorthPyramid = 8,
			SouthPyramid = 9,

			Line = 10,
			Triangle = 11,
			Quadrilateral = 12,
			Pentagon = 13,
			Hexagon = 14,

			Group = 15
		} Type;

		Type getType();

	protected:
		Type type;
		Vector3d origin, size;
};

#include "GeometricObject.h"
#include "Entrance.h"
#include "Sensor.h"
#include "Group.h"

#endif /* defined(__Phantasma__Object__) */
