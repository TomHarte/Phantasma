//
//  VertexAttribute.cpp
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "VertexAttribute.h"
#include "GLHelpers.h"

VertexAttribute::VertexAttribute(GLuint _index, GLint _size, GLenum _type, GLboolean _normalised, std::vector <uint8_t> *_targetPool, std::vector<uint8_t>::size_type _startOffset)
{
	// store the properties that describe this attribute
	attributeIndex = _index;
	attributeSize = _size;
	attributeType = _type;
	attributeIsNormalised = _normalised;

	// store our reference to the target vector
	targetPool = _targetPool;

	// set start offset as required
	startOffset = _startOffset;

	// nominate that we haven't got a buffer for prepared values yet
	preparedValue = nullptr;
}

VertexAttribute::~VertexAttribute()
{
	deleteTemporaryStorage();
}

GLsizei VertexAttribute::sizeOfValue()
{
	// size is the number of components in each item multiplied by the
	// number of bytes per item
	GLsizei numberOfBytes = attributeSize;
	numberOfBytes *= glptSizeOfType(attributeType);

	return numberOfBytes;
}

void VertexAttribute::bindWithStride(GLsizei stride)
{
	// pass the call along
	glVertexAttribPointer(attributeIndex, attributeSize, attributeType, attributeIsNormalised, stride, (void *)startOffset);
}

void VertexAttribute::setValue(const void *value)
{
	GLsizei numberOfBytes = sizeOfValue();

	if(!preparedValue)
	{
		preparedValue = new uint8_t[numberOfBytes];
	}

	memcpy(preparedValue, value, (size_t)numberOfBytes);
}

void VertexAttribute::commitValue()
{
	// copy in as many bytes as make one item; as elsewhere
	// I'm sure this is demonstrating thicky usage of vector
	GLsizei numberOfBytes = sizeOfValue();
	if(preparedValue)
	{
		for(GLsizei byte = 0; byte < numberOfBytes; byte++)
			targetPool->push_back(preparedValue[byte]);
	}
	else
	{
		for(GLsizei byte = 0; byte < numberOfBytes; byte++)
			targetPool->push_back(0);
	}
}

void VertexAttribute::deleteTemporaryStorage()
{
	delete[] preparedValue;
	preparedValue = nullptr;
}
