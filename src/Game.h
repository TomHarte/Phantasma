//
//  Game.h
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Game__
#define __Phantasma__Game__

#include <iostream>

class Game
{
	public:

		Game();

		void setAspectRatio(float aspectRatio);
		void draw();
		void advanceToTime(uint32_t millisecondsSinceArbitraryMoment);

		void setupOpenGL();

	private:
		uint32_t timeOfLastTick;
		bool hasReceivedTime;
};

#endif /* defined(__Phantasma__Game__) */
