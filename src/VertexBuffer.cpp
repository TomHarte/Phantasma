//
//  VertexArray.cpp
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "VertexBuffer.h"
#include <map>

std::map <GLuint, VertexBuffer *> VertexBuffer::boundBuffersMap;

void VertexBuffer::bindAtIndex(GLuint _index)
{
	if(boundBuffersMap[_index] != this)
	{
		if(!buffer)
			glGenBuffers(1, &buffer);

		boundBuffersMap[_index] = this;
		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		glVertexAttribPointer(_index, size, type, normalised, stride, NULL);
	}

	if(bufferIsDirty)
	{
		bufferIsDirty = false;
		glBufferData(GL_ARRAY_BUFFER, (GLsizei)stride*index, &(*targetPool)[startOffset], GL_STATIC_DRAW);
	}
}

VertexBuffer::~VertexBuffer()
{
	if(buffer)
		glDeleteBuffers(1, &buffer);

/*	std::map <GLuint, VertexBuffer *>::iterator mapIterator = boundBuffersMap.begin();
	while(mapIterator != boundBuffersMap.end())
	{
		if(mapIterator->second == this)
			boundBuffersMap[mapIterator->first] = NULL;

		mapIterator++;
	}*/
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
}

size_t VertexBuffer::addValue(const void *value)
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

	const uint8_t *byteValue = (uint8_t *)value;
	while(numberOfBytes--)
	{
		targetPool->push_back(*byteValue);
		byteValue++;
	}

	std::cout << "Pool size now: " << targetPool->size() << std::endl;

	bufferIsDirty = true;
	return returnIndex;
}
