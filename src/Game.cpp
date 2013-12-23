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
#include "VertexBuffer.h"

static float angle = 0.0f;
static VertexBuffer *positionBuffer, *colourBuffer;

Game::Game()
{
	hasReceivedTime = false;

	std::shared_ptr<std::vector <uint8_t>> targetPool(new std::vector <uint8_t>);

	positionBuffer	= new VertexBuffer(4, GL_FLOAT,	GL_FALSE,	32, 0, targetPool);
	colourBuffer	= new VertexBuffer(4, GL_FLOAT,	GL_FALSE,	32, 16, targetPool);

	const GLfloat vertices[] =
	{
		-1.0f,	-1.0f,	10.0f, 1.0f,
		1.0f,	-1.0f,	10.0f, 1.0f,
		-1.0f,	1.0f,	10.0f, 1.0f,
		1.0f,	1.0f,	10.0f, 1.0f,
	};
	const GLfloat colours[] =
	{
		1.0f,	0.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	1.0f, 1.0f,
		0.0f,	1.0f,	1.0f, 1.0f,
	};
//	glVertexAttribPointer(ObjectGLAttributeColour, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)billboardColourData);
//	GLshort vertices[] =
//	{
//		-1, -1, 10,
//		1, -1, 10,
//		-1, 1, 10,
//		1, 1, 10
//	};
//	GLubyte colours[] =
//	{
//		255, 0, 0,
//		255, 255, 0,
//		255, 255, 255,
//		0, 255, 255
//	};

	positionBuffer->addValue(&vertices[0]);
	colourBuffer->addValue(&colours[0]);
	positionBuffer->addValue(&vertices[3]);
	colourBuffer->addValue(&colours[3]);
	positionBuffer->addValue(&vertices[6]);
	colourBuffer->addValue(&colours[6]);
	positionBuffer->addValue(&vertices[9]);
	colourBuffer->addValue(&colours[9]);
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

//	positionBuffer->bindAtIndex()
	Object::drawTestObject(positionBuffer, colourBuffer);
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
