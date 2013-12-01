#include "ebgf_VertexArray.h"
#include <stdlib.h>
#include "SDL.h"
#include "ebgf_GLExts.h"

CVertexArray::CVertexArray(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, int number)
{
	ptr = NULL;

	this->size = size;
	this->type = type;
	this->stride = stride;

	int basesize;
	switch(type)
	{
		case GL_SHORT: basesize = sizeof(GLshort); break;
		case GL_INT: basesize = sizeof(GLint); break;
		case GL_FLOAT: basesize = sizeof(GLfloat); break;
		case GL_DOUBLE: basesize = sizeof(GLdouble); break;
	}

	DataSize = (basesize*size + stride) * number;
	ptr = malloc(DataSize);
	memcpy(ptr, pointer, DataSize);
	Restore();

	Filename = new char[50];
	sprintf(Filename, "VArray::%016x", (int)this);
	__EBGF_StoreHash(this);
}

CVertexArray::~CVertexArray()
{
	Backup();
	free(ptr);
	EBGF_ReturnResource(this, false);
}

void CVertexArray::Backup()
{
	if(buffer)
		glDeleteBuffersARB(1, &buffer);
	buffer = 0;
}

bool CVertexArray::Restore()
{
	buffer = 0;
	if(AvailableGLExtensions&GLEXT_VBO)
	{
		//VBO extension supported, try to upload
		glGenBuffersARB(1, &buffer);
		if(glIsBufferARB(buffer))
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, DataSize, ptr, GL_STATIC_DRAW_ARB);
		}
		else
			buffer = 0;
	}
	return true;
}

void CVertexArray::Enable()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	if(!buffer)
		glVertexPointer(size, type, stride, ptr);
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
		glVertexPointer(size, type, stride, NULL);
	}
}
