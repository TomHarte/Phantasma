//
//  SyntaxBuilder.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__InstructionBuilder__
#define __Phantasma__InstructionBuilder__

#include <vector>
#include <string>
#include "Instruction.h"

std::vector<FCLInstruction *> *getInstructions(std::string *sourceCode);

#endif /* defined(__Phantasma__InstructionBuilder__) */
