//
//  Object.cpp
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Object.h"

Object::Type Object::getType()	{	return type;		}
uint16_t Object::getObjectID()	{	return objectID;	}
Vector3d Object::getOrigin()	{	return origin;		}
Vector3d Object::getSize()		{	return size;		}

void Object::setupOpenGL(VertexBuffer *areaBuffer)		{}
void Object::draw(VertexBuffer *areaBuffer)				{}
