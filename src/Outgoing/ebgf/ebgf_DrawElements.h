#ifndef __EBGF_DRAWELEMENTS_H
#define __EBGF_DRAWELEMENTS_H

#include "SDL_opengl.h"
#include "ebgf_ResourceStore.h"

class CDrawElements: public CResource
{
	public:
		CDrawElements(GLenum mode, GLsizei count, GLenum type, GLvoid *pointer, bool copy = true);
		~CDrawElements();
		void Draw();

		virtual void Backup();
		virtual bool Restore();
	private:
		GLenum mode;
		GLsizei count;
		GLenum type;
		GLvoid *ptr;
		GLuint buffer;
		int DataSize;
};

#endif
