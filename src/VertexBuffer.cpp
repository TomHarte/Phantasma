//
//  VertexArray.cpp
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "VertexBuffer.h"
#include <map>

VertexBuffer *VertexBuffer::boundBuffer;

VertexBuffer::VertexBuffer()
{
	// create a vector to write new values to and seed our stride as 0
	targetPool = std::shared_ptr<std::vector <uint8_t>>(new std::vector <uint8_t>);
	stride = 0;

	// the first write index will be zero
	writeIndex = 0;
}

VertexBuffer::~VertexBuffer()
{
	// cleanup is: returning the OpenGL object, deleting all the attributes
	if(buffer) glDeleteBuffers(1, &buffer);

	for(std::vector <VertexAttribute *>::size_type index = 0; index < attributes.size(); index++)
		delete attributes[index];
}

void VertexBuffer::bind()
{
	// make sure this buffer is allocated and bound
	bool needsBinding = false;
	if(boundBuffer != this)
	{
		boundBuffer = this;
		needsBinding = true;

		if(!buffer) glGenBuffers(1, &buffer);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}

	// make sure we've uploaded the latest data
	if(uploadedLength != targetPool->size())
	{
		if(!uploadedLength)
		{
			glBufferData(GL_ARRAY_BUFFER, (GLsizei)targetPool->size(), &(*targetPool)[0], GL_STATIC_DRAW);
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, (GLsizei)uploadedLength, (GLsizei)(targetPool->size() - uploadedLength), &(*targetPool)[uploadedLength]);
		}

		uploadedLength = targetPool->size();
	}

	// make sure all attributes are bound
	if(needsBinding)
	{
		for(std::vector <VertexAttribute *>::size_type index = 0; index < attributes.size(); index++)
			attributes[index]->bindWithStride(stride);
	}
}

void VertexBuffer::addAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised)
{
	// allocate an attribute, add it to our list
	VertexAttribute *newAttribute = new VertexAttribute(index, size, type, normalised, targetPool);
	attributes.push_back(newAttribute);

	// stride is the sum of all attribute values, so update it now
	stride += newAttribute->sizeOfValue();

	// also store the new attribute for lookup by index
	attributesByIndex[index] = newAttribute;
}

VertexAttribute *VertexBuffer::attributeForIndex(GLuint index)
{
	// trivial, really, just pass the thing along
	return attributesByIndex[index];
}

size_t VertexBuffer::getNextWriteIndex()
{
	// return the current index and increment it for next time
	size_t returnIndex = writeIndex;
	writeIndex++;
	return returnIndex;
}
