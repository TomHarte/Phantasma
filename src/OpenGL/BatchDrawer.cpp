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
	// perform an implicit flush if necessary
	if(
		hasDrawOngoing &&
		(
			(vertexBuffer != currentVertexBuffer) ||
			(drawElementsBuffer != currentDrawElementsBuffer) ||
			(currentMode != mode) ||
			(currentType != type) ||
			(currentPolygonOffset != polygonOffset) ||
			(startIndex + ((size_t)currentCount * glptSizeOfType(type)) != _startIndex)
		)
	)
	{
		flush();
	}

	// if we don't have a batch started, start one;
	// otherwise extend the one we have
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
	}
	else
	{
		currentCount += count;
	}
}

void BatchDrawer::flush()
{
	if(hasDrawOngoing)
	{
		currentVertexBuffer->bind();
		currentDrawElementsBuffer->bind();

		glPolygonOffset(currentPolygonOffset, currentPolygonOffset);
		glDrawElements(currentMode, currentCount, currentType, (void *)startIndex);

		hasDrawOngoing = false;
	}
}
