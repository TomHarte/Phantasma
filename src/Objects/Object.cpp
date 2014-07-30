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

void Object::setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer)					{}
void Object::draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer, BatchDrawer *batchDrawer, bool allowPolygonOffset)	{}
bool Object::isDrawable()								{	return false;	}
bool Object::isPlanar()									{	return false;	}

Object::~Object()				{}

DrawOrder Object::oneWayDrawOrderFromOfComparedTo(Vector3d &position, Object &object, Object &otherObject)
{
	DrawOrder axisOrder[3] = {DrawOrderUnknown, DrawOrderUnknown, DrawOrderUnknown};

	/*

		In Freescape, the axis-aligned bounding boxes of objects are not
		permitted to overlap.

		So the test applied is:

			if there is any axis that separates the two objects but the nominated
			position is between the two objects then it definitely doesn't matter
			what order they're drawn in. They can't overlap. So return DrawOrderDoesntMatter.

			otherwise if there was any axis that separated them and the position was
			definitively not in between the two, figure out the draw order based on
			that axis.

	*/
	for(int axis = 0; axis < 3; axis++)
	{
		// test only if this object is beyond the other object;
		// that's why this is a one-way test
		if(
			object.origin[axis] >= otherObject.origin[axis] + otherObject.size[axis]
		)
		{
			// assume the position is between the two objects,
			// unless we're about to learn otherwise...
			axisOrder[axis] = DrawOrderDoesntMatter;

			// this object is beyond the other and the position is
			// beyond the start of this object, hence the other needs
			// to be drawn first
			if(position[axis] > object.origin[axis])
				axisOrder[axis] = DrawOrderOtherFirst;

			// this object is beyond the other and the position is
			// before the end of the other object, hence this object
			// needs to be drawn first
			if(position[axis] <= otherObject.origin[axis] + otherObject.size[axis])
				axisOrder[axis] = DrawOrderThisFirst;

			// there was a separation but the position was within the area
			// between the two so the draw order doesn't matter
			if(axisOrder[axis] == DrawOrderDoesntMatter)
				return DrawOrderDoesntMatter;
		}
	}

	// either the draw order matters, in which case return it...
	for(int axis = 0; axis < 3; axis++)
		if(axisOrder[axis] != DrawOrderUnknown) return axisOrder[axis];

	// ... or observe that didn't manage to answer the question
	return DrawOrderUnknown;
}

DrawOrder Object::drawOrderFromComparedTo(Vector3d &position, Object &otherObject)
{
	// compose this test of two one-way tests, with the slight caveat that...
	DrawOrder drawOrder = Object::oneWayDrawOrderFromOfComparedTo(position, *this, otherObject);
	if(drawOrder != DrawOrderUnknown) return drawOrder;

	// ... if the draw order is determined from comparing the objects the wrong way
	// around then we need to flip the result
	drawOrder = Object::oneWayDrawOrderFromOfComparedTo(position, otherObject, *this);
	switch(drawOrder)
	{
		default:						return drawOrder;
		case DrawOrderThisFirst:		return DrawOrderOtherFirst;
		case DrawOrderOtherFirst:		return DrawOrderThisFirst;
	}
}
