//
//  Token.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Token.h"

uint32_t Token::GetValue(CGameState *GameState, uint32_t SuggestedValue)
{
	if(Type == CONSTANT) return Value;
	if(Type == VARIABLE) return GameState->GetVariable(Value);
	return SuggestedValue;
}
