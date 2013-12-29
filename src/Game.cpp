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

	position[0] = 1000.0f;
	position[1] = 300.0f;
	position[2] = 1000.0f;

	velocity[0] =
	velocity[1] =
	velocity[2] = 0.0f;
}

Game::~Game()
{
	delete areasByAreaID;
}

void Game::setAspectRatio(float aspectRatio)
{
	// create a projection matrix
	Matrix projectionMatrix = Matrix::projectionMatrix(60.0f, aspectRatio, 1.0f, 100000.0f);
	GeometricObject::setProjectionMatrix(projectionMatrix.contents);
}

void Game::draw()
{
	// just clear the display to a salmon colour
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GeometricObject::setViewMatrix((rotationMatrix * translationMatrix).contents);

	(*areasByAreaID)[1]->draw(position);
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
	float velocityMultiplier = (float)timeDifference;
	position[0] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[0] + velocity[2] * rotationMatrix.contents[2]);
	position[1] -= velocityMultiplier * velocity[1];
	position[2] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[8] + velocity[2] * rotationMatrix.contents[10]);

//	position[0] += velocityMultiplier * (velocity[0] * rotationMatrix.contents[0] + velocity[1] * rotationMatrix.contents[1] + velocity[2] * rotationMatrix.contents[2]);
//	position[1] += velocityMultiplier * (velocity[0] * rotationMatrix.contents[4] + velocity[1] * rotationMatrix.contents[5] + velocity[2] * rotationMatrix.contents[6]);
//	position[2] += velocityMultiplier * (velocity[0] * rotationMatrix.contents[8] + velocity[1] * rotationMatrix.contents[9] + velocity[2] * rotationMatrix.contents[10]);

	translationMatrix = Matrix::translationMatrix(-position[0], -position[1], -position[2]);

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

	Matrix xRotationMatrix = Matrix::rotationMatrix(rotation[0], 1.0f, 0.0f, 0.0f);
	Matrix yRotationMatrix = Matrix::rotationMatrix(rotation[1], 0.0f, 1.0f, 0.0f);
	Matrix zRotationMatrix = Matrix::rotationMatrix(rotation[2], 0.0f, 0.0f, 1.0f);
	rotationMatrix = zRotationMatrix * xRotationMatrix * yRotationMatrix;
}

void Game::setMovementVelocity(float x, float y, float z)
{
	velocity[0] = x;
	velocity[1] = y;
	velocity[2] = z;
}
