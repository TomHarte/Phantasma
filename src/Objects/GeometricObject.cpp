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
#include "BatchDrawer.h"

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
		case Rectangle:
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
	class FaceAdder
	{
		public:
			FaceAdder(VertexBuffer *_vertexBuffer, DrawElementsBuffer *_drawElementsBuffer)
			{
				vertexBuffer = _vertexBuffer;
				drawElementsBuffer = _drawElementsBuffer;
				positionAttribute = vertexBuffer->attributeForIndex(ObjectGLAttributePosition);
				colourAttribute = vertexBuffer->attributeForIndex(ObjectGLAttributeColour);

				drawElementsStartIndex = drawElementsBuffer->getCurrentIndex();

				drawElementsCount = 0;
			}

			void beginFace() //uint8_t red, uint8_t green, uint8_t blue)
			{
				uint8_t red = (uint8_t)arc4random_uniform(256);
				uint8_t green = (uint8_t)arc4random_uniform(256);
				uint8_t blue = (uint8_t)arc4random_uniform(256);

				colour[0] = red;
				colour[1] = green;
				colour[2] = blue;
				faceIndices.clear();
			}

			void addVertex(Vector3d vertex)
			{
				addVertex(vertex.x, vertex.y, vertex.z);
			}

			void addVertex(uint16_t x, uint16_t y, uint16_t z)
			{
				colourAttribute->setValue(colour);

				uint16_t position[3] = {x, y, z};
				positionAttribute->setValue(position);

				faceIndices.push_back((GLushort)vertexBuffer->commitVertex());
			}

			void endFace()
			{
				if(faceIndices.size() < 2)
					return;

				if(faceIndices.size() > 2)
				{
					drawElementsMode = GL_TRIANGLES;
					
					for(std::vector<GLushort>::size_type index = 1; index < (faceIndices.size() - 1); index++)
					{
						drawElementsBuffer->addIndex(&faceIndices[0]);
						drawElementsBuffer->addIndex(&faceIndices[index]);
						drawElementsBuffer->addIndex(&faceIndices[index+1]);
						drawElementsCount += 3;
					}
				}
				else
				{
					drawElementsBuffer->addIndex(&faceIndices[0]);
					drawElementsBuffer->addIndex(&faceIndices[1]);
					drawElementsCount += 2;
					drawElementsMode = GL_LINES;
				}
			}

			size_t drawElementsStartIndex;
			GLsizei drawElementsCount;
			GLenum drawElementsMode;

		private:
			VertexBuffer *vertexBuffer;
			DrawElementsBuffer *drawElementsBuffer;
			VertexAttribute *positionAttribute, *colourAttribute;

			uint8_t colour[3];
			std::vector<GLushort> faceIndices;

	} faceAdder(areaVertexBuffer, areaDrawElementsBuffer);

	switch(this->getType())
	{
		default:
			faceAdder.beginFace();

				faceAdder.addVertex(origin.x,			origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z + size.z);
				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z + size.z);

			faceAdder.endFace();

			faceAdder.beginFace();

				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z);
				faceAdder.addVertex(origin.x,			origin.y,				origin.z);

			faceAdder.endFace();

			faceAdder.beginFace();

				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z);

			faceAdder.endFace();

			faceAdder.beginFace();

				faceAdder.addVertex(origin.x,			origin.y,				origin.z);
				faceAdder.addVertex(origin.x,			origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z + size.z);
				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z);

			faceAdder.endFace();

			faceAdder.beginFace();

				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x,			origin.y,				origin.z + size.z);
				faceAdder.addVertex(origin.x,			origin.y,				origin.z);

			faceAdder.endFace();

			faceAdder.beginFace();

				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z);
				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z + size.z);
				faceAdder.addVertex(origin.x + size.x,	origin.y + size.y,		origin.z);

			faceAdder.endFace();

		break;
		
		case Line:
			faceAdder.beginFace();

				faceAdder.addVertex(origin.x,			origin.y,				origin.z);
				faceAdder.addVertex(origin.x,			origin.y + size.y,		origin.z + size.z);

			faceAdder.endFace();
		break;
		
		case EastPyramid:
		case WestPyramid:
		case UpPyramid:
		case DownPyramid:
		case NorthPyramid:
		case SouthPyramid:
		{
			Vector3d vertices[8] =
			{
				origin,		origin,		origin,		origin,
				origin,		origin,		origin,		origin,
			};

			switch(this->getType())
			{
				default: break;

				case EastPyramid:

					vertices[4].x += size.x;	vertices[5].x += size.x;
					vertices[6].x += size.x;	vertices[7].x += size.x;

					vertices[0].z += size.z;	vertices[1].z += size.z;
					vertices[1].y += size.y;	vertices[2].y += size.y;


					vertices[4].y += (*ordinates)[0];	vertices[4].z += (*ordinates)[3];
					vertices[5].y += (*ordinates)[2];	vertices[5].z += (*ordinates)[3];
					vertices[6].y += (*ordinates)[2];	vertices[6].z += (*ordinates)[1];
					vertices[7].y += (*ordinates)[0];	vertices[7].z += (*ordinates)[1];

				break;
				
				case WestPyramid:

					vertices[0].x += size.x;	vertices[1].x += size.x;
					vertices[2].x += size.x;	vertices[3].x += size.x;

					vertices[1].y += size.y;	vertices[2].y += size.y;
					vertices[2].z += size.z;	vertices[3].z += size.z;

					vertices[4].y += (*ordinates)[0];	vertices[4].z += (*ordinates)[1];
					vertices[5].y += (*ordinates)[2];	vertices[5].z += (*ordinates)[1];
					vertices[6].y += (*ordinates)[2];	vertices[6].z += (*ordinates)[3];
					vertices[7].y += (*ordinates)[0];	vertices[7].z += (*ordinates)[3];

				break;
				
				case UpPyramid:
					vertices[4].y += size.y;	vertices[5].y += size.y;
					vertices[6].y += size.y;	vertices[7].y += size.y;

					vertices[1].x += size.x;	vertices[2].x += size.x;
					vertices[2].z += size.z;	vertices[3].z += size.z;

					vertices[4].x += (*ordinates)[0];	vertices[4].z += (*ordinates)[1];
					vertices[5].x += (*ordinates)[2];	vertices[5].z += (*ordinates)[1];
					vertices[6].x += (*ordinates)[2];	vertices[6].z += (*ordinates)[3];
					vertices[7].x += (*ordinates)[0];	vertices[7].z += (*ordinates)[3];
				break;

				case DownPyramid:
					vertices[0].y += size.y;	vertices[1].y += size.y;
					vertices[2].y += size.y;	vertices[3].y += size.y;

					vertices[0].x += size.x;	vertices[3].x += size.x;
					vertices[2].z += size.z;	vertices[3].z += size.z;

					vertices[4].x += (*ordinates)[2];	vertices[4].z += (*ordinates)[1];
					vertices[5].x += (*ordinates)[0];	vertices[5].z += (*ordinates)[1];
					vertices[6].x += (*ordinates)[0];	vertices[6].z += (*ordinates)[3];
					vertices[7].x += (*ordinates)[2];	vertices[7].z += (*ordinates)[3];
				break;

				case NorthPyramid:
					vertices[4].z += size.z;	vertices[5].z += size.z;
					vertices[6].z += size.z;	vertices[7].z += size.z;

					vertices[0].y += size.y;	vertices[1].y += size.y;
					vertices[1].x += size.x;	vertices[2].x += size.x;

					vertices[4].x += (*ordinates)[0];	vertices[4].y += (*ordinates)[3];
					vertices[5].x += (*ordinates)[2];	vertices[5].y += (*ordinates)[3];
					vertices[6].x += (*ordinates)[2];	vertices[6].y += (*ordinates)[1];
					vertices[7].x += (*ordinates)[0];	vertices[7].y += (*ordinates)[1];
				break;

				case SouthPyramid:
					vertices[0].z += size.z;	vertices[1].z += size.z;
					vertices[2].z += size.z;	vertices[3].z += size.z;

					vertices[1].x += size.x;	vertices[2].x += size.x;
					vertices[2].y += size.y;	vertices[3].y += size.y;

					vertices[4].x += (*ordinates)[0];	vertices[4].y += (*ordinates)[1];
					vertices[5].x += (*ordinates)[2];	vertices[5].y += (*ordinates)[1];
					vertices[6].x += (*ordinates)[2];	vertices[6].y += (*ordinates)[3];
					vertices[7].x += (*ordinates)[0];	vertices[7].y += (*ordinates)[3];
				break;
			}

			faceAdder.beginFace();
				faceAdder.addVertex(vertices[5]); faceAdder.addVertex(vertices[6]); faceAdder.addVertex(vertices[2]); faceAdder.addVertex(vertices[1]);
			faceAdder.endFace();
			faceAdder.beginFace();
				faceAdder.addVertex(vertices[7]); faceAdder.addVertex(vertices[4]); faceAdder.addVertex(vertices[0]); faceAdder.addVertex(vertices[3]);
			faceAdder.endFace();
			faceAdder.beginFace();
				faceAdder.addVertex(vertices[4]); faceAdder.addVertex(vertices[5]); faceAdder.addVertex(vertices[1]); faceAdder.addVertex(vertices[0]);
			faceAdder.endFace();
			faceAdder.beginFace();
				faceAdder.addVertex(vertices[6]); faceAdder.addVertex(vertices[7]); faceAdder.addVertex(vertices[3]); faceAdder.addVertex(vertices[2]);
			faceAdder.endFace();
			faceAdder.beginFace();
				faceAdder.addVertex(vertices[0]); faceAdder.addVertex(vertices[1]); faceAdder.addVertex(vertices[2]); faceAdder.addVertex(vertices[3]);
			faceAdder.endFace();
			faceAdder.beginFace();
				faceAdder.addVertex(vertices[7]); faceAdder.addVertex(vertices[6]); faceAdder.addVertex(vertices[5]); faceAdder.addVertex(vertices[4]);
			faceAdder.endFace();

		}
		break;

		case Triangle:
		case Quadrilateral:
		case Pentagon:
		case Hexagon:
		{

			faceAdder.beginFace();

				for(std::vector<uint16_t>::size_type index = 0; index < ordinates->size(); index += 3)
				{
					faceAdder.addVertex(origin.x + (*ordinates)[index],	 origin.y + (*ordinates)[index + 1],	origin.z + (*ordinates)[index + 2]);
				}

			faceAdder.endFace();

			faceAdder.beginFace();

				for(std::vector<uint16_t>::size_type index = ordinates->size(); index > 0; index -= 3)
				{
					faceAdder.addVertex(origin.x + (*ordinates)[index - 3],	origin.y + (*ordinates)[index - 2],	origin.z + (*ordinates)[index - 1]);
				}

			faceAdder.endFace();
		}
		break;

//		default: break;
	}

	drawElementsCount = faceAdder.drawElementsCount;
	drawElementsStartIndex = faceAdder.drawElementsStartIndex;
	drawElementsMode = faceAdder.drawElementsMode;
}

void GeometricObject::draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer, BatchDrawer *areaBatchDrawer, bool allowPolygonOffset)
{
	areaBatchDrawer->drawElements(areaVertexBuffer, areaDrawElementsBuffer, drawElementsMode, drawElementsCount, GL_UNSIGNED_SHORT, drawElementsStartIndex, (allowPolygonOffset && this->isPlanar()) ? -1.0f : 0.0f);
	return;

	areaVertexBuffer->bind();
	areaDrawElementsBuffer->bind();

	// a triangle strip isn't a helpful option, despite the pure geometrics, because each
	// face may be coloured differently and therefore each vertex may have a different
	// colour depending which face it belongs to — if you stripped and joined with degenerate
	// triangles you'd end up with an array of indices exactly as long as just supplying the
	// triangles directly — four per face plus two two transition to the next face
	glDrawElements(drawElementsMode, drawElementsCount, GL_UNSIGNED_SHORT, (void *)drawElementsStartIndex);
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
bool GeometricObject::isPlanar()
{
	Type type = this->getType();
	return (type >= Object::Line) || !size.x || !size.y || !size.z;
}
