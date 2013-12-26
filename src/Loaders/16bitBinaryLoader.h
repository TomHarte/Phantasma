//
//  16bitBinaryLoader.h
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___6bitBinaryLoader__
#define __Phantasma___6bitBinaryLoader__

#include <iostream>
#include <vector>
#include <stdint.h>
#include "Game.h"

using namespace std;

Game *load16bitBinary(vector <uint8_t> &);

#endif /* defined(__Phantasma___6bitBinaryLoader__) */
