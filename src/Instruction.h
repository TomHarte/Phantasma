//
//  Instruction.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Instruction__
#define __Phantasma__Instruction__

#include "Token.h"
#include <vector>
class CGameState;

class FCLInstruction
{
	public:
		FCLInstruction(Token::Type type);
		virtual ~FCLInstruction();

		void setArguments(Token *);
		void setArguments(Token *, Token *);
		void setArguments(Token *, Token *, Token *);

		Token::Type getType();

		void getValue(CGameState *gameState, int32_t &);
		void getValue(CGameState *gameState, int32_t &, int32_t &);
		void getValue(CGameState *gameState, int32_t &, int32_t &, int32_t &);

		void setBranches(std::vector<FCLInstruction *> *thenBranch, std::vector<FCLInstruction *> *elseBranch);

	private:
		enum Token::Type type;

		union
		{
			struct
			{
				Token *source, *destination, *option;
			} arguments;

			struct
			{
				std::vector<FCLInstruction *> *thenInstructions, *elseInstructions;
			} conditional;
		};
};


#endif /* defined(__Phantasma__Instruction__) */
