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
		static int numberOfOrdinatesForType(Type type);

		static void setupOpenGL();
		static void setProjectionMatrix(const GLfloat *projectionMatrix);
		static void setViewMatrix(const GLfloat *projectionMatrix);

		static VertexBuffer *newVertexBuffer();
		static DrawElementsBuffer *newDrawElementsBuffer();

		GeometricObject(
			Type type,
			uint16_t objectID,
			const Vector3d &origin,
			const Vector3d &size,
			std::vector<uint8_t> *colours,
			std::vector<uint16_t> *ordinates,
			FCLInstructionVector condition);
		virtual ~GeometricObject();

		void setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer);
		void draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer);
		bool isDrawable();

	private:
		static GLuint openGLProgram;
		static GLuint compileShader(const GLchar *source, GLenum shaderType);
		static GLint viewMatrixUniform, projectionMatrixUniform;

		FCLInstructionVector condition;
		std::shared_ptr<std::vector<uint8_t>> colours;
		std::shared_ptr<std::vector<uint16_t>> ordinates;

		size_t drawElementsStartIndex;
		GLsizei drawElementsCount;
};

#endif /* defined(__Phantasma__GeometricObject__) */
