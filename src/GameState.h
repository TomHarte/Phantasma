//
//  GameState.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__GameState__
#define __Phantasma__GameState__

#include <iostream>

class CGameState
{
	public:
		uint32_t GetVariable(uint32_t variableNumber);
};

#endif /* defined(__Phantasma__GameState__) */
