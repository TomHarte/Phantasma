//
//  VertexAttribute.cpp
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "VertexAttribute.h"

VertexAttribute::VertexAttribute(GLuint _index, GLint _size, GLenum _type, GLboolean _normalised, std::shared_ptr<std::vector <uint8_t>> &_targetPool)
{
	// store the properties that describe this attribute
	attributeIndex = _index;
	attributeSize = _size;
	attributeType = _type;
	attributeIsNormalised = _normalised;

	// store our reference to the target vector
	targetPool = _targetPool;

	// set start offset as uninitalised
	startOffset = (unsigned)-1;
}

GLsizei VertexAttribute::sizeOfValue()
{
	// size is the number of components in each item multiplied by the
	// number of bytes per item
	GLsizei numberOfBytes = attributeSize;
	switch(attributeType)
	{
		default:
		break;

		case GL_UNSIGNED_BYTE:
		case GL_BYTE:				numberOfBytes *= sizeof(GLbyte);	break;

		case GL_UNSIGNED_SHORT:
		case GL_SHORT:				numberOfBytes *= sizeof(GLshort);	break;

		case GL_UNSIGNED_INT:
		case GL_INT:				numberOfBytes *= sizeof(GLint);		break;

		case GL_FLOAT:				numberOfBytes *= sizeof(GLfloat);	break;
		case GL_DOUBLE:				numberOfBytes *= sizeof(GLdouble);	break;
	}

	return numberOfBytes;
}

void VertexAttribute::bindWithStride(GLsizei stride)
{
	// pass the call along
	glVertexAttribPointer(attributeIndex, attributeSize, attributeType, attributeIsNormalised, stride, (void *)startOffset);
}

void VertexAttribute::addValue(const void *value)
{
	// if we haven't cribbed the start offset yet, grab it now
	// by checking out how many bytes have already been written
	if(startOffset == (unsigned)-1)
	{
		startOffset = targetPool->size();
	}

	// copy in as many bytes as make one item; as elsewhere
	// I'm sure this is demonstrating thicky usage of vector
	const uint8_t *byteValue = (uint8_t *)value;
	GLsizei numberOfBytes = sizeOfValue();
	while(numberOfBytes--)
	{
		targetPool->push_back(*byteValue);
		byteValue++;
	}
}
