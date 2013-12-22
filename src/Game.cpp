//
//  Game.cpp
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Game.h"
#include "Object.h"
#include "Matrix.h"

static float angle = 0.0f;

Game::Game()
{
	hasReceivedTime = false;
}

void Game::setAspectRatio(float aspectRatio)
{
	// create a projection matrix
	Matrix projectionMatrix = Matrix::projectionMatrix(60.0f, aspectRatio, 1.0f, 1000.0f);
	Object::setProjectionMatrix(projectionMatrix.contents);
}

void Game::draw()
{
	// just clear the display to a salmon colour
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Matrix rotationMatrix = Matrix::rotationMatrix(angle, 0.0f, 1.0f, 0.0f);
	Object::setViewMatrix(rotationMatrix.contents);

	Object::drawTestObject();
}

void Game::setupOpenGL()
{
	Object::setupOpenGL();
}

void Game::advanceToTime(uint32_t millisecondsSinceArbitraryMoment)
{
	if(!hasReceivedTime)
	{
		timeOfLastTick = millisecondsSinceArbitraryMoment;
		hasReceivedTime = true;
		return;
	}

	// so how many milliseconds is that since we last paid attention?
	uint32_t timeDifference = millisecondsSinceArbitraryMoment - timeOfLastTick;

	// TODO: player movement updates out here
	angle += (float)timeDifference / 100.0f;

	// we'll advance at 50hz, which makes for some easy integer arithmetic here
	while(timeDifference > 20)
	{
		timeOfLastTick += 20;
		timeDifference -= 20;

		// TODO: in-game timed events here
	}
}
