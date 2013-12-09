//
//  Token.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Token.h"

Token::Token(Type _type)
{
	type = _type;
}

Token::Token(std::string *_string)
{
	type = Token::STRINGLITERAL;
	string = _string;
}

Token::Token(Type _type, int32_t _value)
{
	type = _type;
	value = _value;
}

Token::Type Token::getType()
{
	return type;
}

int32_t Token::getValue(CGameState *gameState, int32_t suggestedValue)
{
	switch(type)
	{
		case CONSTANT:	return value;
		case VARIABLE:	return gameState->getVariable(value);
		default:		return suggestedValue;
	}
}

Token::~Token()
{
	if(type == Token::STRINGLITERAL && string)
		delete string;
}
