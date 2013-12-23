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
	targetPool = std::shared_ptr<std::vector <uint8_t>>(new std::vector <uint8_t>);
	stride = 0;
}

VertexBuffer::~VertexBuffer()
{
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
	VertexAttribute *newAttribute = new VertexAttribute(index, size, type, normalised, targetPool);
	attributes.push_back(newAttribute);

	stride += newAttribute->sizeOfValue();
	attributesByIndex[index] = newAttribute;
}

VertexAttribute *VertexBuffer::attributeForIndex(GLuint index)
{
	return attributesByIndex[index];
}
