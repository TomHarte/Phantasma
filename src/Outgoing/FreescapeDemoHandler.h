#ifndef __FREESCAPEDEMOHANDLER_H
#define __FREESCAPEDEMOHANDLER_H

#include "Freescape.h"

class CFSDemoHandler: public CGameHandler
{
	public:
		void DrawHud(CFont *Font, Sint32 *Variables, float Width, float Height);
};

#endif
