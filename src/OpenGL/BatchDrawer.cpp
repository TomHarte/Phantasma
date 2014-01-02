//
//  BatchDrawer.cpp
//  Phantasma
//
//  Created by Thomas Harte on 01/01/2014.
//  Copyright (c) 2014 Thomas Harte. All rights reserved.
//

#include "BatchDrawer.h"
#include "VertexBuffer.h"
#include "DrawElementsBuffer.h"
#include "GLHelpers.h"

BatchDrawer::BatchDrawer()
{
	hasDrawOngoing = false;
}

void BatchDrawer::drawElements(VertexBuffer *vertexBuffer, DrawElementsBuffer *drawElementsBuffer, GLenum mode, GLsizei count, GLenum type, size_t _startIndex, float polygonOffset)
{
//	glPolygonOffset(polygonOffset, polygonOffset);
//
//	vertexBuffer->bind();
//	drawElementsBuffer->bind();
//	glDrawElements(mode, count, type, (void *)_startIndex);
//	return;

	// if we don't have a batch started, just go
	if(!hasDrawOngoing)
	{
		currentVertexBuffer = vertexBuffer;
		currentDrawElementsBuffer = drawElementsBuffer;
		currentMode = mode;
		currentCount = count;
		currentType = type;
		startIndex = _startIndex;
		currentPolygonOffset = polygonOffset;
		hasDrawOngoing = true;

		return;
	}

	// if we've got a type incompatibility, draw and return now
	if(
		(vertexBuffer != currentVertexBuffer) ||
		(drawElementsBuffer != currentDrawElementsBuffer) ||
		(currentMode != mode) ||
		(currentType != type) ||
		(currentPolygonOffset != polygonOffset)
	)
	{
		flush();
		return;
	}

	// if this is an extension of the current list, add it to the end;
	// otherwise draw now and return
	if(startIndex + ((size_t)currentCount * glptSizeOfType(type)) == _startIndex)
	{
		currentCount += count;
	}
	else
	{
		flush();
		return;
	}
}

void BatchDrawer::flush()
{
	if(hasDrawOngoing)
	{
		glPolygonOffset(currentPolygonOffset, currentPolygonOffset);

		currentVertexBuffer->bind();
		currentDrawElementsBuffer->bind();
		glDrawElements(currentMode, currentCount, currentType, (void *)startIndex);
		hasDrawOngoing = false;

//		std::cout << ".";
	}
}
