//
//  DrawElements.cpp
//  Phantasma
//
//  Created by Thomas Harte on 29/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "DrawElementsBuffer.h"
#include "GLHelpers.h"

DrawElementsBuffer *DrawElementsBuffer::boundBuffer;

DrawElementsBuffer::DrawElementsBuffer(GLenum _indexType)
{
	indexType = _indexType;
	buffer = 0;
}

DrawElementsBuffer::~DrawElementsBuffer()
{
	if(buffer)
		glDeleteBuffers(1, &buffer);
}

void DrawElementsBuffer::bind()
{
	// make sure this buffer is allocated and bound
	if(boundBuffer != this)
	{
		boundBuffer = this;

		if(!buffer)
			glGenBuffers(1, &buffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	}

	// make sure we've uploaded the latest data; if we've gained anything
	// new then add it to the pile
	if(uploadedLength != targetPool.size())
	{
		if(!uploadedLength)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)targetPool.size(), &targetPool[0], GL_STATIC_DRAW);
		}
		else
		{
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)uploadedLength, (GLsizei)(targetPool.size() - uploadedLength), &targetPool[uploadedLength]);
		}

		uploadedLength = targetPool.size();
	}
}

int DrawElementsBuffer::addIndex(void *index)
{
	int writeIndex = (int)targetPool.size();

	uint8_t *ptr = (uint8_t *)index;
	size_t size = glptSizeOfType(indexType);
	while(size--)
	{
		targetPool.push_back(*ptr);
		ptr++;
	}

	return writeIndex;
}
