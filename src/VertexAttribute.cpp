
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
	index = _index;
	size = _size;
	type = _type;
	normalised = _normalised;
	targetPool = _targetPool;
}

GLsizei VertexAttribute::sizeOfValue()
{
	GLsizei numberOfBytes = size;
	switch(type)
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
	glVertexAttribPointer(index, size, type, normalised, stride, (void *)startOffset);
}

size_t VertexAttribute::addValue(const void *value)
{
	size_t returnIndex = index;
	index++;

	if(!returnIndex)
		startOffset = targetPool->size();

	const uint8_t *byteValue = (uint8_t *)value;
	GLsizei numberOfBytes = sizeOfValue();
	while(numberOfBytes--)
	{
		targetPool->push_back(*byteValue);
		byteValue++;
	}

	return returnIndex;
}
