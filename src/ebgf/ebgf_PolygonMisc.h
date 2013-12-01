#ifndef __EBGF_POLYGONMISC_H
#define __EBGF_POLYGONMISC_H

#include "SDL_opengl.h"

/*
	returns the signed area of the polygon stored in Verts — stride is the number of additional
	bytes between vertices, over and above two floats for each (x, y) pair
*/
extern void EBGF_SetupPointInTriangleTest(GLfloat *vert1, GLfloat *vert2, GLfloat *vert3);
extern bool EBGF_QueryPointInTriangle(GLfloat *v);
extern GLfloat EBGF_GetTriangleArea(GLfloat *vert1, GLfloat *vert2, GLfloat *vert3);
extern GLfloat EBGF_GetPolygonArea(GLfloat *Verts, int stride, int numberts);
extern void EBGF_Normalise(GLfloat *array);

#endif
