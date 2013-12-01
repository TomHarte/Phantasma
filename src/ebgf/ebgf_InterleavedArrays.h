#ifndef __EBGF_INTERLEAVEDARRAYS_H
#define __EBGF_INTERLEAVEDARRAYS_H

#include "SDL_opengl.h"
#include "ebgf_ResourceStore.h"

class CInterleavedArrays: public CResource
{
	public:
		CInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer, int number);
		~CInterleavedArrays();
		void Enable();

		virtual void Backup();
		virtual bool Restore();
	private:
		GLenum format, caps;
		GLsizei stride;
		GLvoid *ptr;
		GLuint buffer;
		int DataSize;
};

#endif
