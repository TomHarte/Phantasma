//
//  GeometricObject.cpp
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "GeometricObject.h"

#include <string.h>
#include "VertexBuffer.h"

#pragma mark -
#pragma mark Static Getters

int GeometricObject::numberOfColoursForObjectOfType(Type type)
{
	switch(type)
	{
		default:
		case Entrance:
		case Group:
		case Sensor:			return 0;

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

int GeometricObject::numberOfOrdinatesForType(Type type)
{
	switch(type)
	{
		default:
		case Entrance:
		case Group:
		case Sensor:			return 0;

		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:		return 4;

		case Line:
		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:			return 3*(2 + type - Line);
	}
}

#pragma mark -
#pragma mark Static OpenGL bits — basic shader setup

typedef enum
{
	ObjectGLAttributePosition,
	ObjectGLAttributeColour
} ObjectGLAttribute;

GLuint GeometricObject::openGLProgram;
GLint GeometricObject::viewMatrixUniform, GeometricObject::projectionMatrixUniform;

GLuint GeometricObject::compileShader(const GLchar *source, GLenum shaderType)
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

void GeometricObject::setupOpenGL()
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

void GeometricObject::setProjectionMatrix(const GLfloat *projectionMatrix)
{
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
}

void GeometricObject::setViewMatrix(const GLfloat *projectionMatrix)
{
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, projectionMatrix);
}

#pragma mark -
#pragma mark Temporary Test Code

void GeometricObject::drawTestObject(VertexBuffer *areaBuffer)
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

VertexBuffer *GeometricObject::newVertexBuffer()
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

#pragma mark -
#pragma mark Construction/Destruction

GeometricObject::GeometricObject(
	Type _type,
	uint16_t _objectID,
	const Vector3d &_origin,
	const Vector3d &_size,
	std::vector<uint8_t> *_colours,
	std::vector<uint16_t> *_ordinates,
	FCLInstructionVector _condition)
{
	type = _type;
	objectID = _objectID;
	origin = _origin;
	size = _size;

	if(_colours)	colours		= std::shared_ptr<std::vector<uint8_t>>(_colours);
	if(_ordinates)	ordinates	= std::shared_ptr<std::vector<uint16_t>>(_ordinates);
	condition = _condition;
}
