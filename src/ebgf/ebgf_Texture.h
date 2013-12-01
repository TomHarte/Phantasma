#ifndef __EBGF_TEXTURE_H
#define __EBGF_TEXTURE_H

#include "ebgf_ResourceStore.h"
#include "SDL_opengl.h"
#include "SDL.h"

class CTexture: public CResource
{
	public:
		/* for public use */
		virtual void Activate() = 0;
		float GetAspect() {return Aspect;}
		float Width() {return ImgWidth;}
		float Height() {return ImgHeight;}

	protected:
		GLuint TexID;
		float Aspect, ImgWidth, ImgHeight;
		static CTexture *ActiveTex;
};

CTexture *EBGF_GetTexture(const char *fname, bool compress = false);

enum EBGF_TexConversion
{
	EBGFTC_BUMPTONORMAL, EBGFTC_GREYSCALE, EBGFTC_NORMAL
};
void EBGF_SetNextTextureFormat(EBGF_TexConversion filter);

#endif
