//
//  Object.h
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Object__
#define __Phantasma__Object__

#include <iostream>


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
			Hexagon = 14
		} Type;
	
		static int numberOfColoursForObjectOfType(Type type);
		static bool isPyramidType(Type type);
};

#endif /* defined(__Phantasma__Object__) */
