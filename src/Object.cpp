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
		case Entrance:			return 0;
		case Sensor:			return 2;

		case Line:				return 2;

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

int Object::numberOfVerticesForType(Type type)
{
	switch(type)
	{
		default:				return 0;

		case Sensor:			return 2;

		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return 2;

		case Line:
		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:			return type - Line;
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

	// we'll want reverse face removal
	glEnable(GL_CULL_FACE);
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
	areaBuffer->bind();

	GLushort indices[] =
	{
		0, 1, 2,
		2, 1, 3,

		2, 3, 4,
		4, 3, 5,

		4, 5, 6,
		6, 5, 7,

		6, 7, 0,
		0, 7, 1,

		5, 3, 1,
		7, 5, 1,

		0, 2, 4,
		0, 4, 6,
	};
	
	// a triangle strip isn't a helpful option, despite the pure geometrics, because each
	// face may be coloured differently and therefore each vertex may have a different
	// colour depending which face it belongs to — if you stripped and joined with degenerate
	// triangles you'd end up with an array of indices exactly as long as just supplying the
	// triangles directly — four per face plus two two transition to the next face
	glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLushort), GL_UNSIGNED_SHORT, indices);
}

VertexBuffer *Object::newVertexBuffer()
{
	VertexBuffer *newBuffer = new VertexBuffer;

	newBuffer->addAttribute(ObjectGLAttributePosition,	3,	GL_SHORT,			GL_FALSE);
	newBuffer->addAttribute(ObjectGLAttributeColour,	3,	GL_UNSIGNED_BYTE,	GL_TRUE);

	// TEST CODE: put some vertices and colours into the buffer
	const GLshort billboardVertexData[] =
	{
		-1,	-1,	1,
		1,	-1,	1,

		-1,	1,	1,
		1,	1,	1,

		-1,	1,	-1,
		1,	1,	-1,

		-1,	-1,	-1,
		1,	-1,	-1,
	};

	const GLubyte billboardColourData[] =
	{
		255,	0,		0,
		255,	255,	0,
		255,	255,	255,
		0,		255,	255,
		255,	0,		0,
		255,	255,	0,
		255,	255,	255,
		0,		255,	255,
	};

	VertexAttribute *positionAttribute = newBuffer->attributeForIndex(ObjectGLAttributePosition);
	VertexAttribute *colourAttribute = newBuffer->attributeForIndex(ObjectGLAttributeColour);

	for(int c = 0; c < 8; c++)
	{
		positionAttribute->setValue(&billboardVertexData[c*3]);
		colourAttribute->setValue(&billboardColourData[c*3]);
		newBuffer->commitVertex();
	}

	return newBuffer;
}

