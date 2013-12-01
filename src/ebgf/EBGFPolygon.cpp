#include "ebgf_Polygon.h"
#include <stdlib.h>
#include <stdio.h>

int CPolygon::TVCount, CPolygon::TVAllocated;
GLfloat *CPolygon::TVList;
int CPolygon::VPos, CPolygon::VAlloc;
GLuint *CPolygon::VPtr;
GLUtesselator *CPolygon::tesselator;
GLenum CPolygon::CurType;
CPolygon::Plate *CPolygon::PlateWorker;

void __stdcall CPolygon::Begin(GLenum type)
{
	CurType = type;
	VPtr = NULL;
	VPos = VAlloc = 0;
}

void __stdcall CPolygon::End(void)
{
	Plate *NP = new Plate;
	NP->Elements = new CDrawElements(CurType, VPos, GL_UNSIGNED_INT, VPtr);
	NP->Next = PlateWorker;
	PlateWorker = NP;
	free(VPtr); VPtr = NULL;
}

void __stdcall CPolygon::Vertex(void *Data)
{
	if((VPos+1) >= VAlloc)
	{
		VAlloc += 32;
		VPtr = (GLuint *)realloc(VPtr, sizeof(GLuint)*VAlloc);
	}
	VPtr[VPos] = (GLuint)Data;
	VPos++;
}

void __stdcall CPolygon::Combine(GLdouble coords[3], void *vertex_data[4], GLfloat weight[4], void **outData)
{
	*outData = (void *)StoreVert(coords[0], coords[1]);
}

int CPolygon::StoreVert(GLfloat x, GLfloat y)
{
	// ensure storage is available
	if((TVCount+2) >= (TVAllocated-1))
	{
		TVAllocated += 512;
		TVList = (GLfloat *)realloc(TVList, sizeof(GLfloat)*TVAllocated);
	}

	// store vertex
	TVList[TVCount] = x;
	TVList[TVCount+1] = y;

	int r= TVCount; TVCount += 2;
	return r >> 1;
}

void CPolygon::AddVert(GLfloat x, GLfloat y)
{
	int c = StoreVert(x, y);

	// pass to gluTesselate
	GLdouble VD[3] = {x, y, 0};
	gluTessVertex(tesselator, VD, (void *)c);
}

typedef GLvoid (__stdcall *CallBackFunc)();
CPolygon::CPolygon()
{
	// generate tesselator
	tesselator = gluNewTess();

	// prepare vertex list
	TVList = NULL;
	TVAllocated = TVCount = 0;
	PlateWorker = NULL;

	Array = NULL;
	PlateList = NULL;

	// register callbacks
	gluTessCallback(tesselator, GLU_TESS_BEGIN, (CallBackFunc)Begin);
	gluTessCallback(tesselator, GLU_TESS_END, (CallBackFunc)End);
	gluTessCallback(tesselator, GLU_TESS_COMBINE, (CallBackFunc)Combine);
	gluTessCallback(tesselator, GLU_TESS_VERTEX, (CallBackFunc)Vertex);

	// begin polygon
	gluBeginPolygon(tesselator);
	gluTessBeginContour(tesselator); 
}

void CPolygon::NewContour()
{
	gluTessEndContour(tesselator);
	gluTessBeginContour(tesselator); 
}

void CPolygon::Finish()
{
	// end final polygon
	gluTessEndContour(tesselator);
	gluEndPolygon(tesselator);

	// store vertices
	Array = new CVertexArray(2, GL_FLOAT, 0, TVList, TVCount >> 1);

	/* at this point, lots of info is sent to my callbacks */

	// free memory
	free(TVList); TVList = NULL;

	// free tesselator
	gluDeleteTess(tesselator);

	// take copy of plates
	PlateList = PlateWorker; PlateWorker = NULL;
}

CPolygon::~CPolygon()
{
	delete Array;
	while(PlateList)
	{
		Plate *N = PlateList->Next;
		delete PlateList;
		PlateList = N;
	}
}

void CPolygon::Draw()
{
	if(!Array) return;
	Array->Enable();
	Plate *P = PlateList;
	while(P)
	{
		P->Elements->Draw();
		P = P->Next;
	}
}
