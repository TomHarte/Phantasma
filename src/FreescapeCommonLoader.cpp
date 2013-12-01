#include "Freescape.h"
#include <stdio.h>

void CFreescapeGame::MapColour(CFreescapeGame::CColour *C)
{
	C->Col[0] = Palette[C->Entry][0];
	C->Col[1] = Palette[C->Entry][1];
	C->Col[2] = Palette[C->Entry][2];
}

void CFreescapeGame::Set16PaletteGradient(float *Col1, float *Col2)
{
	for(int c = 0; c < 16; c++)
	{
		float ic = (float)c / 15.0f;
		ic = sqrt(ic);
		Palette[c][0] = ic*Col2[0] + (1-ic)*Col1[0];
		Palette[c][1] = ic*Col2[1] + (1-ic)*Col1[1];
		Palette[c][2] = ic*Col2[2] + (1-ic)*Col1[2];
	}
}

bool CFreescapeGame::SetPalette(char *name)
{
	FILE *f = fopen(name, "rb");
	if(!f) return false;

	for(int entry = 0; entry < 256; entry++)
	{
		for(int component = 0; component < 3; component++)
		{
			Palette[entry][component] = (float)(fgetc(f)&63) / 63.0f;
//			if(Palette[entry][component] > 63) Palette[entry][component] &= 31;
		}
	}

	bool res = true;
	if(feof(f)) res = false;

	fclose(f);
	return res;
}

bool CFreescapeGame::SetFont(char *name)
{
	PrintFont = EBGF_GetFont(name);
	return PrintFont ? true : false;
}
