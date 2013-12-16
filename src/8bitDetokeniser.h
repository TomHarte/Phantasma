//
//  8bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___bitDetokeniser__
#define __Phantasma___bitDetokeniser__

#include <iostream>
#include <vector>
#include <stdint.h>

using namespace std;
shared_ptr<string> detokenise8bitCondition(vector <uint8_t> &tokenisedCondition);

#endif /* defined(__Phantasma___bitDetokeniser__) */
