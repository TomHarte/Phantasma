//
//  GLHelpers.c
//  Phantasma
//
//  Created by Thomas Harte on 29/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include <stdio.h>
#include "GLHelpers.h"

size_t glptSizeOfType(GLenum type)
{
	switch(type)
	{
		default:					break;

		case GL_UNSIGNED_BYTE:
		case GL_BYTE:				return sizeof(GLbyte);

		case GL_UNSIGNED_SHORT:
		case GL_SHORT:				return sizeof(GLshort);

		case GL_UNSIGNED_INT:
		case GL_INT:				return sizeof(GLint);

		case GL_FLOAT:				return sizeof(GLfloat);
		case GL_DOUBLE:				return sizeof(GLdouble);

		case GL_2_BYTES:			return 2*sizeof(GLbyte);
		case GL_3_BYTES:			return 3*sizeof(GLbyte);
		case GL_4_BYTES:			return 4*sizeof(GLbyte);
	}

	return sizeof(int);
}
