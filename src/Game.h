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
#include <map>
#include "Area.h"
#include "Matrix.h"

typedef std::map<uint16_t, std::shared_ptr<Area>> AreaMap;
class Game
{
	public:

		Game(AreaMap *areasByAreaID);
		virtual ~Game();

		void setAspectRatio(float aspectRatio);
		void draw();
		void advanceToTime(uint32_t millisecondsSinceArbitraryMoment);

		void setupOpenGL();

		void rotateView(float x, float y, float z);
		void setMovementVelocity(float x, float y, float z);

	private:
		uint32_t timeOfLastTick;
		bool hasReceivedTime;

		AreaMap *areasByAreaID;

		float rotation[3], velocity[3], position[3];
		Matrix rotationMatrix;
		Matrix translationMatrix;
};

#endif /* defined(__Phantasma__Game__) */
