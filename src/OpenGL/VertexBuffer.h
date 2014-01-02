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
#include "VertexAttribute.h"

/*

	To create an interleaved vertex buffer you need to:

		(1)	create a vertex buffer;
		(2)	add all the vertex attributes you intend to store.

	To add new values to the buffer you need to:

		(1)	set values for each of the attributes that you care about;
		(2) commit the vertex, receiving its committed index in return.

	Attributes for which you don't specify a value for any given vertex
	will have an undefined value.

*/

class VertexBuffer
{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		void bind();

		void addAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised);
		VertexAttribute *attributeForIndex(GLuint index);

		size_t commitVertex();

	private:
		std::vector <uint8_t> targetPool;
		std::vector <VertexAttribute *> attributes;
		std::map <GLuint, VertexAttribute *> attributesByIndex;

		GLuint buffer;
		std::vector<uint8_t>::size_type uploadedLength;
		static VertexBuffer *boundBuffer;

		GLsizei stride;

		size_t writeIndex;
};

#endif /* defined(__Phantasma__VertexArray__) */
