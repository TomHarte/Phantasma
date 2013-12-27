//
//  Game.cpp
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Game.h"
#include "GeometricObject.h"
#include "Matrix.h"
#include "VertexBuffer.h"

Game::Game(AreaMap *_areasByAreaID)
{
	hasReceivedTime = false;
	areasByAreaID = _areasByAreaID;

	rotation[0] =
	rotation[1] =
	rotation[2] = 0.0f;
}

Game::~Game()
{
	delete areasByAreaID;
}

void Game::setAspectRatio(float aspectRatio)
{
	// create a projection matrix
	Matrix projectionMatrix = Matrix::projectionMatrix(60.0f, aspectRatio, 1.0f, 10000.0f);
	GeometricObject::setProjectionMatrix(projectionMatrix.contents);
}

void Game::draw()
{
	// just clear the display to a salmon colour
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Matrix xRotationMatrix = Matrix::rotationMatrix(rotation[0], 1.0f, 0.0f, 0.0f);
	Matrix yRotationMatrix = Matrix::rotationMatrix(rotation[1], 0.0f, 1.0f, 0.0f);
	Matrix zRotationMatrix = Matrix::rotationMatrix(rotation[2], 0.0f, 0.0f, 1.0f);
	Matrix rotationMatrix = xRotationMatrix * yRotationMatrix * zRotationMatrix;

	Matrix translationMatrix = Matrix::translationMatrix(-1000.0f, -300.0f, -1000.0f);
	GeometricObject::setViewMatrix((rotationMatrix * translationMatrix).contents);

	(*areasByAreaID)[1]->draw();
}

void Game::setupOpenGL()
{
	GeometricObject::setupOpenGL();

	for(AreaMap::iterator iterator = areasByAreaID->begin(); iterator != areasByAreaID->end(); iterator++)
		iterator->second->setupOpenGL();
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


	// we'll advance at 50hz, which makes for some easy integer arithmetic here
	while(timeDifference > 20)
	{
		timeOfLastTick += 20;
		timeDifference -= 20;

		// TODO: in-game timed events here
	}
}

void Game::rotateView(float x, float y, float z)
{
	rotation[0] -= x;
	rotation[1] -= y;
	rotation[2] -= z;
}
