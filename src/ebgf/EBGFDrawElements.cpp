#include "ebgf_DrawElements.h"
#include <stdlib.h>
#include "SDL.h"
#include "ebgf_GLExts.h"

CDrawElements::CDrawElements(GLenum mode, GLsizei count, GLenum type, GLvoid *pointer, bool copy)
{
	this->mode = mode;
	this->count = count;
	this->type = type;

	int basesize;
	switch(type)
	{
		case GL_UNSIGNED_BYTE: basesize = sizeof(GLbyte); break;
		case GL_UNSIGNED_SHORT: basesize = sizeof(GLushort); break;
		case GL_UNSIGNED_INT: basesize = sizeof(GLuint); break;
	}

	DataSize = basesize*count;
	if(copy)
	{
		ptr = new unsigned char[DataSize];
		memcpy(ptr, pointer, DataSize);
	}
	else
		ptr = pointer;
	Restore();

	Filename = new char[50];
	sprintf(Filename, "DrawElements::%016x", (int)this);
	__EBGF_StoreHash(this);
}

CDrawElements::~CDrawElements()
{
	Backup();
	delete[] (unsigned char *)ptr;
	EBGF_ReturnResource(this, false);
}

void CDrawElements::Backup()
{
	if(AvailableGLExtensions&GLEXT_VBO)
		glDeleteBuffersARB(1, &buffer);
}

bool CDrawElements::Restore()
{
	if(AvailableGLExtensions&GLEXT_VBO)
	{
		//VBO extension supported, upload
		glGenBuffersARB(1, &buffer);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, DataSize, ptr, GL_STATIC_DRAW_ARB);
	}
	return true;
}

void CDrawElements::Draw()
{
	if(!(AvailableGLExtensions&GLEXT_VBO))
		glDrawElements(mode, count, type, ptr);
	else
	{
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
		glDrawElements(mode, count, type, NULL);
	}
}
