#ifndef __EBGF_POLYGON_H
#define __EBGF_POLYGON_H

#include "ebgf_VertexArray.h"
#include "ebgf_DrawElements.h"

#ifndef WIN32
#define __stdcall
#endif

class CPolygon
{
	public:
		CPolygon();
		~CPolygon();

		void Draw();

		void AddVert(GLfloat x, GLfloat y);
		void NewContour();
		void Finish();

	private:
		static void __stdcall Begin(GLenum type);
		static void __stdcall End(void);
		static void __stdcall Combine(GLdouble coords[3], void *vertex_data[4], GLfloat weight[4], void **outData);
		static void __stdcall Vertex(void *Data);
		static int StoreVert(GLfloat x, GLfloat y);

		CVertexArray *Array;
		struct Plate
		{
			Plate() {Elements = NULL; Next = NULL;}
			~Plate() {delete Elements;}
			CDrawElements *Elements;
			Plate *Next;
		} *PlateList;
		static Plate *PlateWorker;

		static GLfloat *TVList;
		static int TVCount, TVAllocated;
		static GLuint *VPtr;
		static int VPos, VAlloc;
		static GLUtesselator *tesselator;
		static GLenum CurType;
};

#endif
