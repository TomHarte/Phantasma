//
//  BatchDrawer.h
//  Phantasma
//
//  Created by Thomas Harte on 01/01/2014.
//  Copyright (c) 2014 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__BatchDrawer__
#define __Phantasma__BatchDrawer__

#include <iostream>

class VertexBuffer;
class DrawElementsBuffer;
class BatchDrawer
{
	public:

		BatchDrawer();

		void drawElements(VertexBuffer *vertexBuffer, DrawElementsBuffer *drawElementsBuffer, GLenum mode, GLsizei count, GLenum type, size_t startIndex, float polygonOffset);
		void flush();

	private:
		bool hasDrawOngoing;

		GLenum currentMode;
		GLsizei currentCount;
		GLenum currentType;
		size_t startIndex;
		VertexBuffer *currentVertexBuffer;
		DrawElementsBuffer *currentDrawElementsBuffer;
		float currentPolygonOffset;
};

#endif /* defined(__Phantasma__BatchDrawer__) */
