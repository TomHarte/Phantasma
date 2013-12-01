#include "ebgf_PolygonMisc.h"
#include <math.h>

GLfloat *__EBGF_PointInTriangle_A, __EBGF_PointInTriangle_V0[2], __EBGF_PointInTriangle_V1[2];
GLfloat __EBGF_PointInTriangle_Dot00, __EBGF_PointInTriangle_Dot01, __EBGF_PointInTriangle_Dot11;
GLfloat __EBGF_PointInTriangle_InvDenom;

void EBGF_SetupPointInTriangleTest(GLfloat *vert1, GLfloat *vert2, GLfloat *vert3)
{
	__EBGF_PointInTriangle_A = vert1;

	__EBGF_PointInTriangle_V0[0] = vert3[0] - vert1[0];
	__EBGF_PointInTriangle_V0[1] = vert3[1] - vert1[2];

	__EBGF_PointInTriangle_V1[0] = vert2[0] - vert1[0];
	__EBGF_PointInTriangle_V1[1] = vert2[1] - vert1[2];

	__EBGF_PointInTriangle_Dot00 = __EBGF_PointInTriangle_V0[0]*__EBGF_PointInTriangle_V0[0] + __EBGF_PointInTriangle_V0[1]*__EBGF_PointInTriangle_V0[1];
	__EBGF_PointInTriangle_Dot01 = __EBGF_PointInTriangle_V0[0]*__EBGF_PointInTriangle_V1[0] + __EBGF_PointInTriangle_V0[1]*__EBGF_PointInTriangle_V1[1];
	__EBGF_PointInTriangle_Dot11 = __EBGF_PointInTriangle_V1[0]*__EBGF_PointInTriangle_V1[0] + __EBGF_PointInTriangle_V1[1]*__EBGF_PointInTriangle_V1[1];

	__EBGF_PointInTriangle_InvDenom = 1.0f / (__EBGF_PointInTriangle_Dot00 * __EBGF_PointInTriangle_Dot11 - __EBGF_PointInTriangle_Dot01 * __EBGF_PointInTriangle_Dot01);
}

bool EBGF_QueryPointInTriangle(GLfloat *vt)
{
	GLfloat Vec[2] = {vt[0] - __EBGF_PointInTriangle_A[0], vt[1] - __EBGF_PointInTriangle_A[1]};
	GLfloat Dot02 = __EBGF_PointInTriangle_V0[0]*Vec[0] + __EBGF_PointInTriangle_V0[1]*Vec[1];
	GLfloat Dot12 = __EBGF_PointInTriangle_V1[0]*Vec[0] + __EBGF_PointInTriangle_V1[1]*Vec[1];
	GLfloat u, v;

	u = (__EBGF_PointInTriangle_Dot11 * Dot02 - __EBGF_PointInTriangle_Dot01 * Dot12) * __EBGF_PointInTriangle_InvDenom;
	v = (__EBGF_PointInTriangle_Dot00 * Dot12 - __EBGF_PointInTriangle_Dot01 * Dot02) * __EBGF_PointInTriangle_InvDenom;

	return (u > 0) && (v > 0) && (u + v < 1);
}


GLfloat EBGF_GetTriangleArea(GLfloat *vert1, GLfloat *vert2, GLfloat *vert3)
{
	return	0.5 * 
			((vert1[0] - vert2[0])*(vert3[1] - vert2[1]) -
			(vert1[1] - vert2[1])*(vert3[0] - vert2[0]));
}

GLfloat EBGF_GetPolygonArea(GLfloat *verts, int stride, int number)
{
	GLfloat Area = 0;
	int Ptr = 0;

	while(Ptr < number)
	{
		GLfloat *V1, *V2;

		V1 = (GLfloat *)(((unsigned char *)verts) + (Ptr * 2 * (sizeof(GLfloat) + stride)));
		V2 = (GLfloat *)(((unsigned char *)verts) + ( ((Ptr+1)%number) * 2 * (sizeof(GLfloat) + stride)));

		Area += V1[0] * V2[1] - V1[1] * V2[0];
		Ptr++;
	}
	return 0.5*Area;
}

void EBGF_Normalise(GLfloat *array)
{
	float l = 1.0f / sqrt(array[0]*array[0] + array[1]*array[1] + array[2]*array[2]);
	array[0] *= l; array[1] *= l; array[2] *= l;
}
