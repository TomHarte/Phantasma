//
//  Parser.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Parser__
#define __Phantasma__Parser__

#include <vector>
#include <string>
#include "Instruction.h"

FCLInstructionVector getInstructions(std::string *sourceCode);

#endif /* defined(__Phantasma__InstructionBuilder__) */
