//
//  16bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___6bitDetokeniser__
#define __Phantasma___6bitDetokeniser__

#include <iostream>
#include <vector>
#include <stdint.h>

using namespace std;
shared_ptr<string> detokenise16bitCondition(vector <uint8_t> &tokenisedCondition);

#endif /* defined(__Phantasma___6bitDetokeniser__) */
