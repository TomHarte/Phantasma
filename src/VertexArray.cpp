//
//  VertexArray.cpp
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "VertexArray.h"
#include <map>

static std::map <GLuint, VertexBuffer *> boundBuffersMap;

void VertexBuffer::bindAtIndex(GLuint _index)
{
	if(boundBuffersMap[_index] != this)
	{
		boundBuffersMap[_index] = this;
		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		if(bufferIsDirty)
		{
			glBufferData(GL_ARRAY_BUFFER, size, &(*targetPool)[startOffset], GL_STATIC_DRAW);
		}

		glVertexAttribPointer(_index, size, type, normalised, stride, NULL);
	}
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &buffer);
}

VertexBuffer::VertexBuffer(GLint _size, GLenum _type, GLboolean _normalised, GLsizei _stride, std::vector<uint8_t>::size_type _startOffset, std::shared_ptr<std::vector <uint8_t>> &_targetPool)
{
	targetPool = _targetPool;
	size = _size;
	type = _type;
	normalised = _normalised;
	stride = _stride;
	startOffset = _startOffset;

	bufferIsDirty = false;
	index = 0;

	glGenBuffers(1, &buffer);
}

size_t VertexBuffer::addValue(uint8_t *value)
{
	size_t returnIndex = index;
	index++;

	size_t numberOfBytes = (size_t)size;
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

	while(numberOfBytes--)
	{
		targetPool->push_back(*value);
		value++;
	}

	bufferIsDirty = true;
	return returnIndex;
}
