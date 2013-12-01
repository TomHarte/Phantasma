#ifndef __EBGF_VERTEXARRAY_H
#define __EBGF_VERTEXARRAY_H

#include "SDL_opengl.h"
#include "ebgf_ResourceStore.h"

class CVertexArray: public CResource
{
	public:
		CVertexArray(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, int number);
		~CVertexArray();
		void Enable();

		virtual void Backup();
		virtual bool Restore();
	private:
		GLint size;
		GLenum type;
		GLsizei stride;
		GLvoid *ptr;
		GLuint buffer;
		int DataSize;
};

#endif
