//
//  GeometricObject.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "GeometricObject.h"

#include <string.h>

#pragma mark -
#pragma mark Static Getters

int GeometricObject::numberOfColoursForObjectOfType(Type type)
{
	switch(type)
	{
		default:
		case Entrance:
		case Group:
		case Sensor:			return 0;

		case Line:				return 2;

		case Rectangle:
		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:			return 2;

		case Cube:
		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return 6;
	}
}

int GeometricObject::numberOfOrdinatesForType(Type type)
{
	switch(type)
	{
		default:
		case Entrance:
		case Group:
		case Rectangle:
		case Sensor:			return 0;

		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return 4;

		case Line:
		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:			return 3*(2 + type - Line);
	}
}


#pragma mark -
#pragma mark Construction/Destruction

GeometricObject::GeometricObject(
	Type _type,
	uint16_t _objectID,
	const Vector3d &_origin,
	const Vector3d &_size,
	std::vector<uint8_t> *_colours,
	std::vector<uint16_t> *_ordinates,
	FCLInstructionVector _condition)
{
	type = _type;
	objectID = _objectID;
	origin = _origin;
	size = _size;

	if(_colours)	colours		= std::shared_ptr<std::vector<uint8_t>>(_colours);
	if(_ordinates)	ordinates	= std::shared_ptr<std::vector<uint16_t>>(_ordinates);
	condition = _condition;
}

GeometricObject::~GeometricObject()
{
}

bool GeometricObject::isDrawable()								{	return true;	}
bool GeometricObject::isPlanar()
{
	Type type = this->getType();
	return (type >= Object::Line) || !size.x || !size.y || !size.z;
}
