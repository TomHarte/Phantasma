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

class VertexBuffer
{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		void bind();

		void addAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised);
		VertexAttribute *attributeForIndex(GLuint index);

	private:
		std::shared_ptr<std::vector <uint8_t>> targetPool;
		std::vector <VertexAttribute *> attributes;
		std::map <GLuint, VertexAttribute *> attributesByIndex;

		GLuint buffer;
		std::vector<uint8_t>::size_type uploadedLength;
		static VertexBuffer *boundBuffer;

		GLsizei stride;
};

#endif /* defined(__Phantasma__VertexArray__) */
