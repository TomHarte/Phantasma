#ifndef __EBGFFONT_H
#define __EBGFFONT_H

#include "ebgf_ResourceStore.h"
#include "ebgf_Polygon.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

class CFont: public CResource
{
	public:
		bool Open(const char *name);

		void Print(const char *fmt, ...);
		float GetWidth(const char *fmt, ...);

		// don't actually have anything that depends on GL state, so...
		void Backup() {};
		bool Restore() {return true;};

	private:
		struct Glyph
		{
			CPolygon *Graphic;
			float Width;
		} Glyphs[128];
		static Glyph *CurGlyph;
		FT_Face face;

		static FT_Pos CurPos[2];
		static void TTF_AddVert(GLfloat x, GLfloat y);
		static int TTF_MoveTo(const FT_Vector *to, void *user);
		static int TTF_AddLine(const FT_Vector *to, void *user);
		static int TTF_AddQuadratic(const FT_Vector *control, const FT_Vector *to, void *user);
		static int TTF_AddCubic(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user);
		void GetChar(int character);

		static bool InContour;
};

CFont *EBGF_GetFont(const char *fname);

#endif
