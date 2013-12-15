//
//  Token.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Token.h"

Token::Token()
{
	type = UNKNOWN;
}

Token::Token(Type _type)
{
	type = _type;
}

Token::Token(std::shared_ptr<std::string> &_string)
{
	type = STRINGLITERAL;
	string = _string;
}

Token::Token(Type _type, int32_t _value)
{
	type = _type;
	value = _value;
}

Token &Token::operator = (const Token &other)
{
	type = other.type;

	if(type == STRINGLITERAL)
		string = other.string;
	else
		value = other.value;

	return *this;
}

Token::Token(const Token &other)
{
	*this = other;
}

Token::Type Token::getType()
{
	return type;
}

int32_t Token::getValue(CGameState &gameState, int32_t suggestedValue)
{
	switch(type)
	{
		case CONSTANT:	return value;
		case VARIABLE:	return gameState.getVariable(value);
		default:		return suggestedValue;
	}
}
