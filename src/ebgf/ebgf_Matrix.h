#ifndef __EBGF_MATRIX_H
#define __EBGF_MATRIX_H

#include "SDL_opengl.h"
#include "ebgf_Vector.h"

enum CM_ConstructionType
{
	CM_UPRIGHT, CM_UPFRONT, CM_RIGHTFRONT
};

class CMatrix
{
	public:
		CMatrix();

		void Rotate(float angle, float x, float y, float z);
		void Translate(float x, float y, float z);
		void Construct(CM_ConstructionType ct, CVector a, CVector b);
		void Construct(CVector side, CVector up, CVector front);
		void Transpose();

		void Multiply();
		void MultiplyInverse();

		GLfloat Contents[16];
};

CVector operator *(CMatrix m, CVector v);

#endif
