//
//  Token.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Token__
#define __Phantasma__Token__

#include <string>
#include <stdint.h>
#include "GameState.h"

struct Token
{
	public:
		uint32_t GetValue(CGameState *GameState, uint32_t SuggestedValue = 0);

		enum Type
		{
			ACTIVATEDQ, ADDVAR, AGAIN, AND, ANDV,
			COLLIDEDQ,
			DELAY, DESTROY, DESTROYEDQ,
			ELSE, END, ENDGAME, ENDIF, EXECUTE,
			GOTO,
			IF, INVIS, INVISQ, INCLUDE,
			LOOP,
			MODE, MOVE, MOVETO,
			NOTV,
			OR, ORV,
			GETXPOS, GETYPOS, GETZPOS, 
			PRINT,
			RESTART, REDRAW, REMOVE,
			SOUND, SETVAR, SHOTQ, START, STARTANIM, STOPANIM, SUBVAR, SYNCSND,
			THEN, TIMERQ, TOGVIS, TRIGANIM,
			UPDATEI,
			VAREQ, VARGQ, VARLQ, VISQ, VIS,
			WAIT, WAITTRIG,
			COMMA, OPENBRACKET, CLOSEBRACKET, CONSTANT, VARIABLE, STRINGLITERAL,
			UNKNOWN, ENDOFFILE,
			SETBIT, CLEARBIT, TOGGLEBIT, SWAPJET, BITNOTEQ, VARNOTEQ
		};

		Token(Type type);

	private:
		Type Type;

		union
		{
			uint32_t Value;
			std::string String;
		};
};


#endif /* defined(__Phantasma__Token__) */