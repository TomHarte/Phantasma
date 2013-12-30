//
//  DrawElementsBuffer.h
//  Phantasma
//
//  Created by Thomas Harte on 29/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef Phantasma_DrawElementsBuffer_h
#define Phantasma_DrawElementsBuffer_h

#include <vector>

class DrawElementsBuffer
{
	public:
		DrawElementsBuffer(GLenum indexType);
		virtual ~DrawElementsBuffer();

		size_t addIndex(void *index);

		void bind();

	private:
		std::vector<uint8_t> targetPool;
		std::vector<uint8_t>::size_type uploadedLength;

		GLuint buffer;
		GLenum indexType;
		static DrawElementsBuffer *boundBuffer;
};

#endif
