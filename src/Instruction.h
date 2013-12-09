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

class FCLInstruction
{
	public:
		FCLInstruction(Token::Type type);
		virtual ~FCLInstruction();

		void setArguments(Token *);
		void setArguments(Token *, Token *);
		void setArguments(Token *, Token *, Token *);
		
		void setBranches(std::vector<FCLInstruction *> *thenBranch, std::vector<FCLInstruction *> *elseBranch);

	private:
		enum Token::Type Type;

		union
		{
			struct
			{
				Token Source, Dest, Other;
			} TernaryOp;

			struct
			{
				Token Source, Dest;
			} BinaryOp;

			std::string String;

			Token UnaryOp;
			struct
			{
				FCLInstruction *Passed, *Failed;
			} Then;
		};
};


#endif /* defined(__Phantasma__Instruction__) */
