//
//  Object.cpp
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "Object.h"
#include <string.h>
#include "VertexBuffer.h"

#pragma mark -
#pragma mark Static Getters

int Object::numberOfColoursForObjectOfType(Type type)
{
	switch(type)
	{
		default:
		case Entrance:
		case Sensor:			return 0;

		case Line:				return 1;

		case Rectangle:
		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:			return 2;

		case Cube:
		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return 6;
	}
}

bool Object::isPyramidType(Type type)
{
	switch(type)
	{
		default:				return false;

		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return true;
	}
}

#pragma mark -
#pragma mark Static OpenGL bits — basic shader setup

typedef enum
{
	ObjectGLAttributePosition,
	ObjectGLAttributeColour
} ObjectGLAttribute;

GLuint Object::openGLProgram;
GLint Object::viewMatrixUniform, Object::projectionMatrixUniform;

GLuint Object::compileShader(const GLchar *source, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if(!status)
	{
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char shaderLog[logLength];
		glGetShaderInfoLog(shader, logLength, NULL, shaderLog);
		std::cerr << shaderLog << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

void Object::setupOpenGL()
{
	// use the shading language version string to decide
	// whether we're on ES or full-fat OpenGL
	const GLubyte *glShadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	if(!glShadingLanguageVersion) return;

	const GLchar *fragmentShaderSource;
	const GLchar *vertexShaderSource;

	if(strlen((char *)glShadingLanguageVersion) >= 9 && glShadingLanguageVersion[8] == 'E' && glShadingLanguageVersion[9] == 'S')
	{
		fragmentShaderSource =
			"varying lowp vec4 colourVarying;"
			""
			"void main()"
			"{"
				"gl_FragColor = colourVarying;"
			"}";

		vertexShaderSource =
			"attribute vec4 position;"
			"attribute vec4 colour;"
			""
			"uniform mat4 view, projection;"
			""
			"varying lowp vec4 colourVarying;"
			""
			"void main()"
			"{"
				"colourVarying = colour;"
				"gl_Position = projection * view * position;"
			"}";
	}
	else
	{
		fragmentShaderSource =
			"varying vec4 colourVarying;"
			""
			"void main()"
			"{"
				"gl_FragColor = colourVarying;"
			"}";

		vertexShaderSource =
			"attribute vec4 position;"
			"attribute vec4 colour;"
			""
			"uniform mat4 view, projection;"
			""
			"varying vec4 colourVarying;"
			""
			"void main()"
			"{"
				"colourVarying = colour;"
				"gl_Position = projection * view * position;"
			"}";
	}

	// compile and link our trivial shader program
	openGLProgram = glCreateProgram();
	GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	glAttachShader(openGLProgram, vertexShader);
	glAttachShader(openGLProgram, fragmentShader);

	// bind the attributes
	glBindAttribLocation(openGLProgram, ObjectGLAttributePosition, "position");
	glBindAttribLocation(openGLProgram, ObjectGLAttributeColour, "colour");

	// link the thing
	glLinkProgram(openGLProgram);

	// grab the uniforms
	viewMatrixUniform = glGetUniformLocation(openGLProgram, "view");
	projectionMatrixUniform = glGetUniformLocation(openGLProgram, "projection");

	// this is actually the only program we're going to use, so we can
	// start using it here
	glUseProgram(openGLProgram);

	// similarly, we'll enable both arrays, since we'll always be supplying both
	glEnableVertexAttribArray(ObjectGLAttributePosition);
	glEnableVertexAttribArray(ObjectGLAttributeColour);

	std::cout << openGLProgram << " " << vertexShader << " " << fragmentShader << " " << viewMatrixUniform << " " << projectionMatrixUniform;
}

void Object::setProjectionMatrix(const GLfloat *projectionMatrix)
{
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
}

void Object::setViewMatrix(const GLfloat *projectionMatrix)
{
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, projectionMatrix);
}

void Object::drawTestObject(VertexBuffer *areaBuffer)
{
/*	const GLfloat billboardVertexData[] =
	{
		-1.0f,	-1.0f,	10.0f, 1.0f,
		1.0f,	-1.0f,	10.0f, 1.0f,
		-1.0f,	1.0f,	10.0f, 1.0f,
		1.0f,	1.0f,	10.0f, 1.0f,
	};
	glVertexAttribPointer(ObjectGLAttributePosition, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)billboardVertexData);

	const GLfloat billboardColourData[] =
	{
		1.0f,	0.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	1.0f, 1.0f,
		0.0f,	1.0f,	1.0f, 1.0f,
	};
	glVertexAttribPointer(ObjectGLAttributeColour, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)billboardColourData);*/

	areaBuffer->bind();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

VertexBuffer *Object::newVertexBuffer()
{
	VertexBuffer *newBuffer = new VertexBuffer;
	
	newBuffer->addAttribute(ObjectGLAttributePosition,	4, GL_FLOAT, GL_FALSE);
	newBuffer->addAttribute(ObjectGLAttributeColour,	3, GL_FLOAT, GL_FALSE);

	// TEST CODE: put some vertices and colours into the buffer
	const GLfloat billboardVertexData[] =
	{
		-1.0f,	-1.0f,	10.0f, 1.0f,
		1.0f,	-1.0f,	10.0f, 1.0f,
		-1.0f,	1.0f,	10.0f, 1.0f,
		1.0f,	1.0f,	10.0f, 1.0f,
	};

	const GLfloat billboardColourData[] =
	{
		1.0f,	0.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	0.0f, 1.0f,
		1.0f,	1.0f,	1.0f, 1.0f,
		0.0f,	1.0f,	1.0f, 1.0f,
	};
	
	VertexAttribute *positionAttribute = newBuffer->attributeForIndex(ObjectGLAttributePosition);
	VertexAttribute *colourAttribute = newBuffer->attributeForIndex(ObjectGLAttributeColour);

	positionAttribute->addValue(&billboardVertexData[0]);
	colourAttribute->addValue(&billboardColourData[0]);

	positionAttribute->addValue(&billboardVertexData[4]);
	colourAttribute->addValue(&billboardColourData[4]);

	positionAttribute->addValue(&billboardVertexData[8]);
	colourAttribute->addValue(&billboardColourData[8]);

	positionAttribute->addValue(&billboardVertexData[12]);
	colourAttribute->addValue(&billboardColourData[12]);

	return newBuffer;
}

