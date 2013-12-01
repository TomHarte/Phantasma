#include "ebgf.h"
#include <typeinfo>
#include <stdarg.h>
#include <stdio.h>

// stuff to create a Freetype library object

FT_Library __ebgf_TTFLibrary = NULL;

void __EBGF_FreeFontLibrary()
{
	FT_Done_FreeType(__ebgf_TTFLibrary);
}

void __EBGF_SetupFontLibrary()
{
	FT_Init_FreeType(&__ebgf_TTFLibrary);
	atexit(__EBGF_FreeFontLibrary);
}

/*

	Font class

*/
#define GLYPH_MULTIPLIER	128

bool CFont::Open(const char *name)
{
	if(FT_New_Face(__ebgf_TTFLibrary, name, 0, &face)) return false;
	FT_Set_Char_Size(face, GLYPH_MULTIPLIER << 6, GLYPH_MULTIPLIER << 6, 72, 72);	// default size is 1x1, with 72dpi
	
	int c = 128;
	while(c--)
		GetChar(c);

	return true;
}

float CFont::GetWidth(const char *fmt, ...)
{
	char String[2048];
	va_list varlist;

	va_start(varlist, fmt);
//	vsnprintf(String, 2048, fmt, varlist);
	vsprintf(String, fmt, varlist);
	va_end(varlist);

	fmt = String;

	float R = 0;
		while(*fmt)
		{
			int idx = (*fmt)&127;
			R += Glyphs[idx].Width;

			int glyph_index1;
			int glyph_index2;
			FT_Vector kerning;
			glyph_index1 = FT_Get_Char_Index(face, (FT_ULong)fmt[0]);
			glyph_index2 = FT_Get_Char_Index(face, (FT_ULong)fmt[1]);
			FT_Get_Kerning(face, glyph_index1, glyph_index2, FT_KERNING_UNFITTED, &kerning);
			R += kerning.x;
			fmt++;
		}

	return R / (64.0f * GLYPH_MULTIPLIER);
}

void CFont::Print(const char *fmt, ...)
{
	char String[2048];
	va_list varlist;

	va_start(varlist, fmt);
//	vsnprintf(String, 2048, fmt, varlist);
	vsprintf(String, fmt, varlist);
	va_end(varlist);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	fmt = String;

	glPushMatrix();
		glScalef(1.0f / (64.0f * GLYPH_MULTIPLIER), 1.0f / (64.0f * GLYPH_MULTIPLIER), 1.0f / (64.0f * GLYPH_MULTIPLIER));
		while(*fmt)
		{
			int idx = (*fmt)&127;
			Glyphs[idx].Graphic->Draw();
			glTranslatef(Glyphs[idx].Width, 0, 0);

			int glyph_index1;
			int glyph_index2;
			FT_Vector kerning;
			glyph_index1 = FT_Get_Char_Index(face, (FT_ULong)fmt[0]);
			glyph_index2 = FT_Get_Char_Index(face, (FT_ULong)fmt[1]);
			FT_Get_Kerning(face, glyph_index1, glyph_index2, FT_KERNING_UNFITTED, &kerning);
			glTranslatef(kerning.x, 0, 0);
			fmt++;
		}
	glPopMatrix();
	glPopAttrib();
}

bool CFont::InContour;
FT_Pos CFont::CurPos[2];
CFont::Glyph *CFont::CurGlyph;

void CFont::TTF_AddVert(GLfloat x, GLfloat y)
{
	CurGlyph->Graphic->AddVert(x, y);
}

int CFont::TTF_MoveTo(const FT_Vector *to, void *user)
{
	/* pen up, move, pen down — breaks polygon */
	if(!InContour || CurPos[0] != to->x || CurPos[1] != to->y)
	{
		if(InContour)
			CurGlyph->Graphic->NewContour();
		InContour = true;

		CurPos[0] = to->x;
		CurPos[1] = to->y;
		TTF_AddVert(CurPos[0], CurPos[1]);
	}

	return 0;
}

int CFont::TTF_AddLine(const FT_Vector *to, void *user)
{
	/* move */
	if(CurPos[0] != to->x || CurPos[1] != to->y)
		TTF_AddVert(CurPos[0] = to->x, CurPos[1] = to->y);
	return 0;
}

#define BEZIER_SEGMENTS		3

int CFont::TTF_AddQuadratic(const FT_Vector *control, const FT_Vector *to, void *user)
{
	/* quadratic bezier from current location to 'to', with control point 'control' */
	
	for(int c = 1; c < BEZIER_SEGMENTS; c++)
	{
		float t = (float)c / BEZIER_SEGMENTS;
		
		float coeff1 = (1-t) * (1-t);
		float coeff2 = 2 * t * (1-t);
		float coeff3 = t * t;

		TTF_AddVert(
			coeff1*CurPos[0] + coeff2 * control->x + coeff3*to->x,
			coeff1*CurPos[1] + coeff2 * control->y + coeff3*to->y
		);
	}
	TTF_AddVert(CurPos[0] = to->x, CurPos[1] = to->y);
	return 0;
}

int CFont::TTF_AddCubic(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
{
	/* cubic bezier - rarely happens */
	
	for(int c = 1; c < BEZIER_SEGMENTS; c++)
	{
		float t = (float)c / BEZIER_SEGMENTS;

		float coeff1 = (1-t) * (1-t) * (1-t);
		float coeff2 = 3 * t * (1-t) * (1-t);
		float coeff3 = 3 * t * t;
		float coeff4 = t * t * t;

		TTF_AddVert(
			coeff1*CurPos[0] + coeff2 * control1->x + coeff3*control2->x + coeff4*to->x,
			coeff1*CurPos[1] + coeff2 * control1->y + coeff3*control2->y + coeff4*to->y
		);
	}
	TTF_AddVert(CurPos[0] = to->x, CurPos[1] = to->y);
	return 0;
}

void CFont::GetChar(int character)
{
	FT_Outline outline;
    FT_Outline_Funcs func_interface;

    func_interface.shift = 0;
    func_interface.delta = 0;
    func_interface.move_to = TTF_MoveTo;
    func_interface.line_to = TTF_AddLine;
    func_interface.conic_to = TTF_AddQuadratic;
    func_interface.cubic_to = TTF_AddCubic;

    /* lookup glyph */
	if(FT_Load_Char(face, character, FT_LOAD_NO_BITMAP|FT_LOAD_NO_HINTING)) return;
	outline = face->glyph->outline;

	CurPos[0] = CurPos[1] = 0;
	CurGlyph = &Glyphs[character];
	CurGlyph->Graphic = new CPolygon;

	InContour = false;

	// throw font shape into tesselator
	FT_Outline_Decompose( &outline, &func_interface, NULL);
	Glyphs[character].Width = face->glyph->advance.x;
	CurGlyph->Graphic->Finish();
}

/*

	EXPORTED STUFF, for actually getting hold of a font object

*/

CFont *__EBGF_GetFont(const char *fname)
{
	CFont *NewFnt = new CFont;
	if(!NewFnt) return NULL;
	if(!NewFnt->Open(fname))
	{
		delete NewFnt;
		return NULL;
	}
	return NewFnt;
}

CFont *EBGF_GetFont(const char *fname)
{
	if(!__ebgf_TTFLibrary)
		__EBGF_SetupFontLibrary();
	else
	{
		/* see if this can be located */
		CResource *Search = __EBGF_FindFirstResource(fname);
		while(Search)
		{
			if( typeid(*Search) == typeid(CFont) )
			{
				Search->RefCount++;
				return (CFont *)Search;
			}
			Search = __EBGF_FindNextResource(fname);
		}
	}
	
	// couldn't find resource, make a new one
	CFont *Fnt = __EBGF_GetFont(fname);
	if(!Fnt) return NULL;

	/* if we managed to get the texture, store the filename and return it */
	Fnt->Filename = strdup(fname);
	__EBGF_StoreHash(Fnt);

	return Fnt;
}
