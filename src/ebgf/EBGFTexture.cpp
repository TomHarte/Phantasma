#include "ebgf_Texture.h"
#include "SDL.h"
#include "SDL_image.h"
#include "ebgf_GLExts.h"
#include <typeinfo>
#include <math.h>

CTexture *CTexture::ActiveTex = NULL;
typedef GLint (* EBGF_TextureProcessor)(Uint8 *Data, GLint format, int w, int h);
EBGF_TextureProcessor __EBGF_ProcFunc = NULL;

class CUncompStaticTex: public CTexture
{
	public:
		CUncompStaticTex();
		~CUncompStaticTex();

		void Activate();

		void Backup();
		bool Restore();
		bool Open(const char *name);

	private:
		int Width, Height;
		bool AlphaChannel;
		Uint8 *BackupData;
};

class CCompStaticTex: public CTexture
{
	public:
		CCompStaticTex() {ProcFunc = NULL;}
		void Activate();

		void Backup();
		bool Restore();
		bool Open(const char *name);

	private:
		EBGF_TextureProcessor ProcFunc;
};

/*
	TEXTURE LIST
*/
CTexture *__EBGF_GetTexture(const char *fname, bool compress)
{
	if(compress && (AvailableGLExtensions&GLEXT_TEXCOMPRESS))
	{
		CCompStaticTex *NewTex = new CCompStaticTex;
		if(NewTex->Open(fname))
			return NewTex;
		delete NewTex;

		return NULL;
	}
	else
	{
		CUncompStaticTex *NewTex = new CUncompStaticTex;
		if(NewTex->Open(fname))
			return NewTex;
		delete NewTex;

		return NULL;
	}
}

CTexture *EBGF_GetTexture(const char *fname, bool compress)
{
	/* see if this can be located */
	CResource *Search = __EBGF_FindFirstResource(fname);
	while(Search)
	{
		if( 
				(compress && typeid(*Search) == typeid(CCompStaticTex)) ||
				(!compress && typeid(*Search) == typeid(CUncompStaticTex))
			)
		{
			Search->RefCount++;
			return (CTexture *)Search;
		}
		Search = __EBGF_FindNextResource(fname);
	}

	/* okay, doesn't seem to be already loaded, so try to get texture */
	CTexture *Tex = __EBGF_GetTexture(fname, compress);
	if(!Tex){
		EBGF_SetNextTextureFormat(EBGFTC_NORMAL);
		return NULL;
	}

	/* if we managed to get the texture, store the filename and return it */
	if(!Tex->Filename) Tex->Filename = strdup(fname);
	__EBGF_StoreHash(Tex);

	return Tex;
}

/*
	Helper functions for all that follows...
*/

void __EBGF_getrgba(SDL_Surface *surface, int x, int y, Uint8 &R, Uint8 &G, Uint8 &B, Uint8 &A)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	Uint32 Colour;

	switch(bpp)
	{
		default:
		case 1:	
			Colour = *(Uint8 *)p; break;
		case 2:		Colour = *(Uint16 *)p; break;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				Colour = p[0] << 16 | p[1] << 8 | p[2];
			else
				Colour = p[0] | p[1] << 8 | p[2] << 16;
		break;
		case 4:
			Colour = *(Uint32 *)p;
		break;
	}
	SDL_GetRGBA(Colour, surface->format, &R, &G, &B, &A);
}

Uint8 *__EBGF_GetDataFromSurface(SDL_Surface *surf, bool &Alpha, int &NewWidth, int &NewHeight)
{
	Alpha = surf->format->Amask ? true : false;
	bool AlphaTrick = false;

	/* might be alpha, even if no alpha channel - check for hot pink or expansion */
	if(!Alpha)
	{
		for(int y = 0; y < surf->h; y++)
			for(int x = 0; x < surf->w; x++)
			{
				Uint8 R, G, B, A;
				__EBGF_getrgba(surf, x, y, R, G, B, A);
				if(R == B && R == 0xff && !G)
				{
					Alpha = AlphaTrick = true;
					break;
				}
			}
	}

	/* round width and height up to a power of 2 */
	NewWidth = 1;
	while(NewWidth < surf->w) NewWidth <<= 1;
	NewHeight = 1;
	while(NewHeight < surf->h) NewHeight <<= 1;

	if(surf->h != NewHeight || surf->w != NewWidth) Alpha = true;

	/* allocate storage and convert */
	Uint8 *Ret = new Uint8[NewWidth * NewHeight * (Alpha ? 4 : 3)];
	int XOff, YOff;
	XOff = (NewWidth - surf->w) >> 1;
	YOff = NewHeight - surf->h;

	memset(Ret, 0, NewWidth * NewHeight * (Alpha ? 4 : 3));

	SDL_LockSurface(surf);
	for(int y = 0; y < surf->h; y++)
		for(int x = 0; x < surf->w; x++)
		{
			Uint8 *Base = &Ret[ (((y + YOff) * NewWidth) + x + XOff) * (Alpha ? 4 : 3)];
			Uint8 R, G, B, A;
			__EBGF_getrgba(surf, x, y, R, G, B, A);
			Base[0] = R;
			Base[1] = G;
			Base[2] = B;
			if(Alpha)
			{
				Base[3] = AlphaTrick ? 0xff : A;
				if(AlphaTrick && R == B && R == 0xff && !G)
					Base[3] = 0;
			}
		}
	SDL_UnlockSurface(surf);

	return Ret;
}

/*
	UNCOMPRESSED, STATIC TEXTURE

	Loads texture from disc and uploads it to texture memory. When it needs to backup, it downloads it back to RAM.
	To restore, it just reuploads to texture memory.
*/
CUncompStaticTex::CUncompStaticTex()
{
	BackupData = NULL;
	Width = Height = 0;
}

CUncompStaticTex::~CUncompStaticTex()
{
	if(BackupData)
		delete[] BackupData;
	else
		glDeleteTextures(1, &TexID);
}

void CUncompStaticTex::Activate()
{
	if(ActiveTex != this)
	{
		ActiveTex = this;
		glBindTexture(GL_TEXTURE_2D, TexID);
	}
}

bool CUncompStaticTex::Open(const char *name)
{
	SDL_Surface *surf = IMG_Load(name);

	if(surf)
	{
		/* lock surface */
		BackupData = __EBGF_GetDataFromSurface(surf, AlphaChannel, Width, Height);
		ImgWidth = Width;
		ImgHeight = Height;
		Aspect = (float)Width / Height;

		/* process, if relevant */
		if(__EBGF_ProcFunc)
		{
			__EBGF_ProcFunc(BackupData, AlphaChannel ? GL_RGBA : GL_RGB, ImgWidth, ImgHeight);
			__EBGF_ProcFunc = NULL;
		}

		/* upload, free memory */
		Restore();

		/* unlock surface */
		SDL_UnlockSurface(surf);

		/* free surface */
		SDL_FreeSurface(surf);

		/* success! */
		return true;
	}

	return false;
}

void CUncompStaticTex::Backup()
{
	Activate();
	BackupData = new Uint8[Width*Height*(AlphaChannel ? 4 : 3)];
	glGetTexImage(GL_TEXTURE_2D, 0, AlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, BackupData);
	glDeleteTextures(1, &TexID);
}

bool CUncompStaticTex::Restore()
{
	glGenTextures(1, &TexID);
	Activate();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	gluBuild2DMipmaps(GL_TEXTURE_2D, AlphaChannel ? GL_RGBA : GL_RGB, Width, Height, AlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, BackupData);
	delete[] BackupData;
	BackupData = NULL;
	return true;
}

/*
	COMPRESSED, STATIC TEXTURE

	Loads texture from disc and uploads it to texture memory. When it needs to backup, it releases the video memory.
	To restore, it reloads the texture from disc.
*/
void CCompStaticTex::Activate()
{
	if(ActiveTex != this)
	{
		ActiveTex = this;
		glBindTexture(GL_TEXTURE_2D, TexID);
	}
}

bool CCompStaticTex::Open(const char *name)
{
	ProcFunc = __EBGF_ProcFunc;
	__EBGF_ProcFunc = NULL;
	Filename = strdup(name);
	return Restore();
}

bool CCompStaticTex::Restore()
{
	SDL_Surface *surf = IMG_Load(Filename);

	if(surf)
	{
		/* upload, free memory */
		glGenTextures(1, &TexID);
		Activate();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		int Width, Height;
		bool AlphaChannel;
		Uint8 *BackupData =__EBGF_GetDataFromSurface(surf, AlphaChannel, Width, Height);
		/* process, if relevant */
		if(ProcFunc)
			ProcFunc(BackupData, AlphaChannel ? GL_RGBA : GL_RGB, ImgWidth, ImgHeight);
		gluBuild2DMipmaps(GL_TEXTURE_2D, AlphaChannel ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB, Width, Height, AlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, BackupData);
		delete[] BackupData;

		ImgWidth = (float)Width;
		ImgHeight = (float)Height;
		Aspect = (float)Width / Height;

		/* unlock surface */
		SDL_UnlockSurface(surf);

		/* free surface */
		SDL_FreeSurface(surf);

		/* success! */
		return true;
	}

	return false;
}

void CCompStaticTex::Backup()
{
	glDeleteTextures(1, &TexID);
}

void __EBGF_TF_AddNormalVec(GLfloat *V, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	// get vectors
	x3 -= x2; y3 -= y2; z3 -= z2;
	x2 -= x1; y2 -= y1; z2 -= z1;

	// get cross product, normalise
	GLfloat Normal[3], len;
	Normal[0] = y2*z3 - y3*z2;
	Normal[1] = z2*x3 - z3*x2;
	Normal[2] = x2*y3 - x3*y2;
	
	len = 1.0f / sqrt(Normal[0]*Normal[0] + Normal[1]*Normal[1] + Normal[2]*Normal[2]);
	V[0] -= Normal[0]*len;
	V[1] -= Normal[1]*len;
	V[2] -= Normal[2]*len;
}

GLint __EBGF_TP_GreyScale(Uint8 *Data, GLint format, int w, int h)
{
	// TODO: allow for alpha
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			Uint8 r, g, b;
			r = Data[ (((y*w) + x)*3) ];
			g = Data[ (((y*w) + x)*3) +1];
			b = Data[ (((y*w) + x)*3) +2];
			
			Data[ (y*w)+x ] = (19595 * r + 38470 * g + 7471 * b) >> 16;
		}
	}
	return GL_LUMINANCE;
}

GLint __EBGF_TP_BumpToNormal(Uint8 *Data, GLint format, int w, int h)
{
	//TODO: allow for alpha
	__EBGF_TP_GreyScale(Data, format, w, h);

	Uint8 *Output = new Uint8[w*h*3];
	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			// process pixel at (x, y)
			GLfloat Heights[5], SumVec[3];

			Heights[0] = (float)Data[((((y-1+h)%h)*w) + x)] / 255.0f;
			Heights[1] = (float)Data[((((y+1)%h)*w) + x)] / 255.0f;

			Heights[2] = (float)Data[((y*w) + ((x-1+w)%w) )] / 255.0f;
			Heights[3] = (float)Data[((y*w) + ((x+1)%w) )] / 255.0f;

			Heights[4] = (float)Data[((y*w) + x)] / 255.0f;

			SumVec[0] = SumVec[1] = SumVec[2] = 0;
			// sum four normals
			__EBGF_TF_AddNormalVec(SumVec, -1, 0, Heights[2], 0, 0, Heights[4], 0, -1, Heights[0]);
			__EBGF_TF_AddNormalVec(SumVec, 0, -1, Heights[0], 0, 0, Heights[4], 1, 0, Heights[3]);
			__EBGF_TF_AddNormalVec(SumVec, 1, 0, Heights[3], 0, 0, Heights[4], 0, 1, Heights[1]);
			__EBGF_TF_AddNormalVec(SumVec, 0, 1, Heights[1], 0, 0, Heights[4], -1, 0, Heights[2]);

			// divide by 4
			SumVec[0] /= 4;
			SumVec[1] /= 4;
			SumVec[2] /= 4;

			Output[ (((y*w) + x) * 3) + 0] = 128 + (int)(SumVec[0] * 127.0f);
			Output[ (((y*w) + x) * 3) + 1] = 128 + (int)(SumVec[1] * 127.0f);
			Output[ (((y*w) + x) * 3) + 2] = 128 + (int)(SumVec[2] * 127.0f);
		}
	}
	memcpy(Data, Output, w*h*3);
	delete[] Output;
	return GL_RGB;
}

void EBGF_SetNextTextureFormat(EBGF_TexConversion filter)
{
	switch(filter)
	{
		case EBGFTC_NORMAL:
			__EBGF_ProcFunc = NULL;
		break;
		case EBGFTC_GREYSCALE:
			__EBGF_ProcFunc = __EBGF_TP_GreyScale;
		break;
		case EBGFTC_BUMPTONORMAL:
			__EBGF_ProcFunc = __EBGF_TP_BumpToNormal;
		break;
	}
}
