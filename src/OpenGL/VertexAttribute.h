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
		VertexAttribute(GLuint index, GLint size, GLenum type, GLboolean normalised, std::vector <uint8_t> *targetPool, std::vector<uint8_t>::size_type startOffset);
		virtual ~VertexAttribute();

		void setValue(const void *value);
		void bindWithStride(GLsizei stride);

		GLsizei sizeOfValue();

	private:
		std::vector<uint8_t> *targetPool;
		std::vector<uint8_t>::size_type startOffset;

		uint8_t *preparedValue;

		GLuint attributeIndex;
		GLint attributeSize;
		GLenum attributeType;
		GLboolean attributeIsNormalised;

		friend class VertexBuffer;
		void commitValue();
		void deleteTemporaryStorage();
};

#endif /* defined(__Phantasma__VertexAttribute__) */
