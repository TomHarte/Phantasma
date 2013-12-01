#ifndef __EBGF_OBJECT_H
#define __EBGF_OBJECT_H

#include "ebgf_Texture.h"
#include "ebgf_InterleavedArrays.h"
#include "ebgf_DrawElements.h"
#include "ebgf_ResourceStore.h"
#include "ebgf_Shader.h"
#include "ebgf_Vector.h"
#include "ebgf_Matrix.h"
#include "SDL_opengl.h"

class CMaterial: public CResource
{
	public:
		CMaterial();
		~CMaterial();
		void Backup() {}
		bool Restore();
		void Activate(GLfloat *TextureSpaceMatrices);

		void MakeShader();

		// these are read and understood
		CTexture *AmbientMap, *DiffuseMap, *SpecularMap, *NormalMap;
		GLfloat AmbientColour[4];
		GLfloat DiffuseColour[4];
		GLfloat SpecularColour[4];
		GLfloat Dissolve;
		GLfloat SpecularExponent;
		int IlluminationModel;

		// these are read and ignored (for now)
		GLfloat OpticalDensity;

		enum
		{
			PLAIN,

			DIFFUSE, DIFFUSE_BUMP, DIFFUSE_SPECULAR, DIFFUSE_SPECULAR_BUMP
		} ShaderMode;
		CShaderPair *Shader;

		GLint MatrixSlot, DiffuseSlot, NormalSlot, SpecularSlot, AmbientSlot;

		// these are for the future
/*		GLfloat TransmissionFilter[3];
		int IlluminationModel;
		bool Halo;
		GLfloat ReflectiveSharpness;*/
};

class CObject: public CResource
{
	public:
		CObject();
		~CObject();
		void Backup() {};
		bool Restore() {return true;};

		void Draw();
		bool Open(const char *filename);

	private:
		/* work up to an GL_T2F_N3F_V3F interleaved array */
		template <class T>
		class DataAdder
		{
			public:
				DataAdder() {Array = NULL; NumAllocated = NumStored = 0;}
				~DataAdder() {Flush();}
				void Flush();

				void Add(T v);
				T Get(int index) {return Array[index];}
				int GetNum() {return NumStored;}
				T *GetArray() {return Array;}
				T &operator [](int n) {return &Array[n];}

			private:
				T *Array;
				int NumAllocated, NumStored;
		};
		DataAdder<GLfloat> *Positions, *TexCoords, *Normals, *ArrayDA, *TextureSpaceMatrices;
		DataAdder<GLuint> *ElementsDA;

		struct IAPointer
		{
			int VertexNum, NormalNum, TextureNum;
			int ArrayEntry;
			IAPointer *Next;
		} * IAList[65536];
		void InitIAList();
		void ClearIAList();
		int GetIA(int VertexNum, int NormalNum, int TextureNum);

		CInterleavedArrays *Verts;
		struct MaterialGroup
		{
			MaterialGroup() {Material = NULL; Elements = NULL; Next = NULL;}
			CMaterial *Material;
			CDrawElements *Elements;
			MaterialGroup *Next;
		} *Segments;

		void AddTextureSpaceMatrix(int V1, int V2, int V3);
		void FixLists();
};

// returns the material with that prefix if it has already been loaded, otherwise loads all 
bool EBGF_ReadMaterials(const char *prefix, const char *file);
CMaterial *EBGF_GetMaterial(const char *prefix, const char *material);
CObject *EBGF_GetObject(const char *file);

#endif
