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

	static float angle = 0.0f;
	Matrix rotationMatrix = Matrix::rotationMatrix(angle, 0.0f, 1.0f, 0.0f);
	angle += 1.0f;
	Object::setViewMatrix(rotationMatrix.contents);

	Object::drawTestObject();
}

void Game::setupOpenGL()
{
	Object::setupOpenGL();
}
