#include "ebgf_InterleavedArrays.h"
#include <stdlib.h>
#include "SDL.h"
#include "ebgf_GLExts.h"

CInterleavedArrays::CInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer, int number)
{
	this->format = format;
	this->stride = stride;

	int basesize;
	switch(format)
	{
		case GL_V2F:				basesize = 2*sizeof(GLfloat);						break;
		case GL_V3F:				basesize = 3*sizeof(GLfloat);						break;
		case GL_C4UB_V2F:			basesize = 2*sizeof(GLfloat) + 4*sizeof(GLubyte);	break;
		case GL_N3F_V3F:			basesize = 6*sizeof(GLfloat);						break;
		case GL_C3F_V3F:			basesize = 6*sizeof(GLfloat);						break;
		case GL_C4F_N3F_V3F:		basesize = 10*sizeof(GLfloat);						break;
		case GL_T2F_V3F:			basesize = 5*sizeof(GLfloat);						break;
		case GL_T4F_V4F:			basesize = 8*sizeof(GLfloat);						break;
		case GL_T2F_C4UB_V3F:		basesize = 5*sizeof(GLfloat) + 4*sizeof(GLubyte);	break;
		case GL_T2F_C3F_V3F:		basesize = 8*sizeof(GLfloat);						break;
		case GL_T2F_N3F_V3F:		basesize = 8*sizeof(GLfloat);						break;
		case GL_T2F_C4F_N3F_V3F:	basesize = 12*sizeof(GLfloat);						break;
		case GL_T4F_C4F_N3F_V4F:	basesize = 15*sizeof(GLfloat);						break;
	}

	DataSize = (basesize + stride) * number;
	ptr = malloc(DataSize);
	memcpy(ptr, pointer, DataSize);
	Restore();

	Filename = new char[50];
	sprintf(Filename, "InterleavedArray::%016x", (int)this);
	__EBGF_StoreHash(this);
}

CInterleavedArrays::~CInterleavedArrays()
{
	Backup();
	free(ptr);
	EBGF_ReturnResource(this, false);
}

void CInterleavedArrays::Backup()
{
	if(AvailableGLExtensions&GLEXT_VBO)
		glDeleteBuffersARB(1, &buffer);
}

bool CInterleavedArrays::Restore()
{
	if(AvailableGLExtensions&GLEXT_VBO)
	{
		//VBO extension supported, upload
		glGenBuffersARB(1, &buffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, DataSize, ptr, GL_STATIC_DRAW_ARB);
	}
	return true;
}

void CInterleavedArrays::Enable()
{
	if(!(AvailableGLExtensions&GLEXT_VBO))
		glInterleavedArrays(format, stride, ptr);
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
		glInterleavedArrays(format, stride, NULL);
	}
}
