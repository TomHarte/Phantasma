//
//  SyntaxBuilder.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__SyntaxBuilder__
#define __Phantasma__SyntaxBuilder__

#include <vector>
#include <string>
#include "Instruction.h"

std::vector<FCLInstruction> *GetInstructions(std::string source);

#endif /* defined(__Phantasma__SyntaxBuilder__) */
