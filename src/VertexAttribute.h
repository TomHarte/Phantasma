//
//  VertexAttribute.h
//  Phantasma
//
//  Created by Thomas Harte on 22/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__VertexAttribute__
#define __Phantasma__VertexAttribute__

#include <iostream>
#include <vector>

class VertexAttribute
{
	public:
		VertexAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised, std::shared_ptr<std::vector <uint8_t>> &targetPool);

		size_t addValue(const void *value);
		void bindWithStride(GLsizei stride);

		GLsizei sizeOfValue();

	private:
		std::shared_ptr<std::vector <uint8_t>> targetPool;
		std::vector<uint8_t>::size_type startOffset;

		GLuint index;
		GLint size;
		GLenum type;
		GLboolean normalised;
};

#endif /* defined(__Phantasma__VertexAttribute__) */
