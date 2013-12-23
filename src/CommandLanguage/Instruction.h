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

class FCLInstruction;
using namespace std;
typedef shared_ptr<vector<FCLInstruction>> FCLInstructionVector;

class FCLInstruction
{
	public:
		FCLInstruction(Token::Type type);
		FCLInstruction(const FCLInstruction &source);

		void setArguments(Token &);
		void setArguments(Token &, Token &);
		void setArguments(Token &, Token &, Token &);

		Token::Type getType();

		void getValue(CGameState &, int32_t &);
		void getValue(CGameState &, int32_t &, int32_t &);
		void getValue(CGameState &, int32_t &, int32_t &, int32_t &);

		void setBranches(FCLInstructionVector thenBranch, FCLInstructionVector elseBranch);

	private:
		enum Token::Type type;

		struct
		{
			Token source, destination, option;
		} arguments;

		struct
		{
			FCLInstructionVector thenInstructions, elseInstructions;
		} conditional;
};


#endif /* defined(__Phantasma__Instruction__) */
