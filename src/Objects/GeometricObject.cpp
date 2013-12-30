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
#include "DrawElementsBuffer.h"

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

VertexBuffer *GeometricObject::newVertexBuffer()
{
	VertexBuffer *newBuffer = new VertexBuffer;

	newBuffer->addAttribute(ObjectGLAttributePosition,	3,	GL_UNSIGNED_SHORT,	GL_FALSE);
	newBuffer->addAttribute(ObjectGLAttributeColour,	3,	GL_UNSIGNED_BYTE,	GL_TRUE);

	return newBuffer;
}

DrawElementsBuffer *GeometricObject::newDrawElementsBuffer()
{
	return new DrawElementsBuffer(GL_UNSIGNED_SHORT);
}

#pragma mark -
#pragma mark Rendering


void GeometricObject::setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer)
{
	// get the two attributes
	VertexAttribute *positionAttribute = areaVertexBuffer->attributeForIndex(ObjectGLAttributePosition);
	VertexAttribute *colourAttribute = areaVertexBuffer->attributeForIndex(ObjectGLAttributeColour);

	// populate with a cube of the bounding box; TODO: shape and colour properly
	const GLushort cubeVertexData[] =
	{
		(GLushort)origin.x,				(GLushort)origin.y,				(GLushort)(origin.z + size.z),
		(GLushort)(origin.x + size.x),	(GLushort)origin.y,				(GLushort)(origin.z + size.z),

		(GLushort)origin.x,				(GLushort)(origin.y + size.y),	(GLushort)(origin.z + size.z),
		(GLushort)(origin.x + size.x),	(GLushort)(origin.y + size.y),	(GLushort)(origin.z + size.z),

		(GLushort)origin.x,				(GLushort)(origin.y + size.y),	(GLushort)origin.z,
		(GLushort)(origin.x + size.x),	(GLushort)(origin.y + size.y),	(GLushort)origin.z,

		(GLushort)origin.x,				(GLushort)origin.y,				(GLushort)origin.z,
		(GLushort)(origin.x + size.x),	(GLushort)origin.y,				(GLushort)origin.z,
	};

	const GLubyte cubeColourData[] =
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

	GLushort indices[8];

	for(int c = 0; c < 8; c++)
	{
		positionAttribute->setValue(&cubeVertexData[c*3]);
		colourAttribute->setValue(&cubeColourData[c*3]);
		indices[c] = (GLushort)areaVertexBuffer->commitVertex();
	}

	// push in the draw indices
	GLushort drawindices[] =
	{
		indices[0], indices[1], indices[2],
		indices[2], indices[1], indices[3],

		indices[2], indices[3], indices[4],
		indices[4], indices[3], indices[5],

		indices[4], indices[5], indices[6],
		indices[6], indices[5], indices[7],

		indices[6], indices[7], indices[0],
		indices[0], indices[7], indices[1],

		indices[5], indices[3], indices[1],
		indices[7], indices[5], indices[1],

		indices[0], indices[2], indices[4],
		indices[0], indices[4], indices[6],
	};

	drawElementsCount = sizeof(drawindices) / sizeof(GLushort);
	for(GLsizei index = 0; index < drawElementsCount; index++)
	{
		if(!index)
			drawElementsStartIndex = areaDrawElementsBuffer->addIndex(&drawindices[index]);
		else
			areaDrawElementsBuffer->addIndex(&drawindices[index]);
	}
}

void GeometricObject::draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer)
{
	areaVertexBuffer->bind();
	areaDrawElementsBuffer->bind();

	// a triangle strip isn't a helpful option, despite the pure geometrics, because each
	// face may be coloured differently and therefore each vertex may have a different
	// colour depending which face it belongs to — if you stripped and joined with degenerate
	// triangles you'd end up with an array of indices exactly as long as just supplying the
	// triangles directly — four per face plus two two transition to the next face
	glDrawElements(GL_TRIANGLES, drawElementsCount, GL_UNSIGNED_SHORT, (void *)drawElementsStartIndex);
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

GeometricObject::~GeometricObject()
{
}

bool GeometricObject::isDrawable()								{	return true;	}
