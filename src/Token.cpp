//
//  Token.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Token.h"

uint32_t Token::GetValue(CGameState *gameState, uint32_t suggestedValue)
{
	switch(type)
	{
		case CONSTANT:	return value;
		case VARIABLE:	return gameState->GetVariable(value);
		default:		return suggestedValue;
	}
}
