#ifndef __EBGF_VECTOR_H
#define __EBGF_VECTOR_H

#include "SDL_opengl.h"
#include <math.h>

class CVector
{
	public:
		CVector() {Data[0] = Data[1] = Data[2] = 0;}
		CVector(GLfloat x, GLfloat y, GLfloat z) {Data[0] = x; Data[1] = y; Data[2] = z;}
		CVector(const GLfloat *V) {Data[0] = V[0]; Data[1] = V[1]; Data[2] = V[2];}

		inline const float *operator =(const float *arg) {Data[0] = arg[0]; Data[1] = arg[1]; Data[2] = arg[2]; return arg;}
		inline const double *operator =(const double *arg) {Data[0] = arg[0]; Data[1] = arg[1]; Data[2] = arg[2]; return arg;}

		inline void Normalise()
		{
			float l = 1.0f / sqrt(SQLength());
			Data[0] *= l; Data[1] *= l; Data[2] *= l;
		}

		GLfloat SQLength();

		GLfloat Data[3];
};


CVector operator ^(CVector a, CVector b);
CVector operator -(CVector a, CVector b);
CVector operator +(CVector a, CVector b);
GLfloat operator *(CVector a, CVector b);
CVector operator *(CVector a, GLfloat b);
CVector operator /(CVector a, GLfloat b);

#endif
