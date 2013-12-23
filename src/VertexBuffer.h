//
//  VertexArray.h
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__VertexArray__
#define __Phantasma__VertexArray__

#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>

class VertexBuffer
{
	public:
		VertexBuffer(GLint size, GLenum type, GLboolean normalised, GLsizei stride, std::vector<uint8_t>::size_type startOffset, std::shared_ptr<std::vector <uint8_t>> &targetPool);
		virtual ~VertexBuffer();

		size_t addValue(const void *value);

		void bindAtIndex(GLuint index);

	private:
		std::shared_ptr<std::vector <uint8_t>> targetPool;
		GLint size;
		GLenum type;
		GLboolean normalised;
		GLsizei stride;
		std::vector<uint8_t>::size_type startOffset;

		bool bufferIsDirty;
		size_t index;

		GLuint buffer;

		static std::map <GLuint, VertexBuffer *> boundBuffersMap;
};

#endif /* defined(__Phantasma__VertexArray__) */
