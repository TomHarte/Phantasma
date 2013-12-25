//
//  GeometricObject.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__GeometricObject__
#define __Phantasma__GeometricObject__

#include "Object.h"

class GeometricObject: public Object
{
	public:

		static int numberOfColoursForObjectOfType(Type type);
		static int numberOfVerticesForType(Type type);

		static void setupOpenGL();
		static void setProjectionMatrix(const GLfloat *projectionMatrix);
		static void setViewMatrix(const GLfloat *projectionMatrix);

		static void drawTestObject(VertexBuffer *areaBuffer);

		static VertexBuffer *newVertexBuffer();

		GeometricObject(
			Type type,
			const Vector3d &origin,
			const Vector3d &size,
			const std::shared_ptr<std::vector<uint8_t>> *colours,
			const std::shared_ptr<std::vector<uint16_t>> *vertices,
			FCLInstructionVector *condition);

	private:
		static GLuint openGLProgram;
		static GLuint compileShader(const GLchar *source, GLenum shaderType);
		static GLint viewMatrixUniform, projectionMatrixUniform;

		FCLInstructionVector condition;
		Type type;
		Vector3d origin, size;
		std::shared_ptr<std::vector<uint8_t>> colours;
		std::shared_ptr<std::vector<uint16_t>> vertices;
};

#endif /* defined(__Phantasma__GeometricObject__) */
