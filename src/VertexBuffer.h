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
	Warning! Semantics are a little odd here.

	To create an interleaved vertex buffer you need to:

		(1)	create a vertex buffer;
		(2)	add all the vertex attributes you intend to store.

	To add new values to the buffer you need to:

		-	go through every one of the attributes, storing a new value,
			making sure you go through them in exactly the same order as
			everyone else does.

	You should use getNextWriteIndex to find out which index in the
	buffer you're about to write to but, again, that works only if it
	is used every single time.

*/

class VertexBuffer
{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		void bind();

		void addAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised);
		VertexAttribute *attributeForIndex(GLuint index);

		size_t getNextWriteIndex();

	private:
		std::shared_ptr<std::vector <uint8_t>> targetPool;
		std::vector <VertexAttribute *> attributes;
		std::map <GLuint, VertexAttribute *> attributesByIndex;

		GLuint buffer;
		std::vector<uint8_t>::size_type uploadedLength;
		static VertexBuffer *boundBuffer;

		GLsizei stride;

		size_t writeIndex;
};

#endif /* defined(__Phantasma__VertexArray__) */
