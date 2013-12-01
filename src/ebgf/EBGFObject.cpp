#include "ebgf.h"

#include "ebgf_Object.h"
#include "stdio.h"
#include <typeinfo>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*

	__EBGF_OBJ_GetMaterialName takes 'prefix' and 'material' strings and combines them to make a single name
	for this material. It's useful for ensuring that duplicate materials are only loaded once
	
*/
const char *__EBGF_OBJ_GetMaterialName(const char *prefix, const char *material)
{
	static char MatName[2048];
	sprintf(MatName, "%s::%s", prefix, material);
	return MatName;
}

/*

	__EBGF_OBJ_FindMaterial searches to see if a material with the specified combination of prefix
	and material exists

*/
CMaterial *__EBGF_OBJ_FindMaterial(const char *prefix, const char *material)
{
	const char *MaterialName = __EBGF_OBJ_GetMaterialName(prefix, material);
	CResource *Search = __EBGF_FindFirstResource(MaterialName);
	while(Search)
	{
		if( typeid(*Search) == typeid(CMaterial) )
		{
			return (CMaterial *)Search;
		}
		Search = __EBGF_FindNextResource(MaterialName);
	}
	return NULL;
}

/*

	EBGF_GetMaterial is externally accessible. It searches for the material identified by the
	given prefix/material pair, and returns it if found, incrementing its refcout so it can
	later be returned with ReturnResource.

*/
CMaterial *EBGF_GetMaterial(const char *prefix, const char *material)
{
	// see if it is already loaded
	CMaterial *Search = __EBGF_OBJ_FindMaterial(prefix, material);
	if(Search)
	{
		Search->RefCount++;
		return Search;
	}

	// it isn't
	return NULL;
}

/*

	EBGF_GetObject returns a CObject file of the file specified in 'fname'. It loads all materials
	as necessary.

*/
CObject *EBGF_GetObject(const char *fname)
{
	/* see if this can be located */
	CResource *Search = __EBGF_FindFirstResource(fname);
	while(Search)
	{
		if(typeid(*Search) == typeid(CObject))
		{
			Search->RefCount++;
			return (CObject *)Search;
		}
		Search = __EBGF_FindNextResource(fname);
	}

	/* okay, doesn't seem to be already loaded, so try to get texture */
	CObject *Obj = new CObject;
	if(!Obj->Open(fname))
	{
		delete Obj;
		return NULL;
	}

	/* if we managed to get the texture, store the filename and return it */
	Obj->Filename = strdup(fname);
	__EBGF_StoreHash(Obj);

	return Obj;
}

/*

	A couple of helper functions.

	__EBGF_OBJ_GetDirectory returns the directory portion of the filename in 'file'.
	__EBGF_OBJ_SpliceDirectory appends 'child' to the directory portion of the filename in 'file'.

*/
const char *__EBGF_OBJ_GetDirectory(const char *file)
{
	static char PathStr[2048];
	strcpy(PathStr, file);
	char *P = PathStr + strlen(PathStr);
	while(P != PathStr && *P != '/')
		P--;
	P[1] = '\0';
	return PathStr;
}

const char *__EBGF_OBJ_SpliceDirectory(const char *file, const char *child)
{
	static char PathStr[2048];
	strcpy(PathStr, file);
	char *P = PathStr + strlen(PathStr);
	while(P != PathStr && *P != '/')
		P--;
	strcpy(&P[1], child);
	return PathStr;
}

/*

	A __EBGF_OBJ_CParser is a class that parses a subset of OBJ/MDL files.

	At the minute, the code will have at most two of these in use at once. It really needs to just use one
	and follow mtllib/etc as preprocessing directives. But it doesn't yet!

*/

class __EBGF_OBJ_CParser
{
	public:
		enum Commands
		{
			UNKNOWN, COMMENT, ENDOFLINE, 

			NEWMATERIAL, USEMATERIAL, LOADMATERIALS,

			SPECULAREXPONENT, DISSOLVE, OPTICALDENSITY,

			ILLUMINATIONMODEL,

			AMBIENTCOLOUR, DIFFUSECOLOUR, SPECULARCOLOUR,

			AMBIENTMAP, DIFFUSEMAP, SPECULARMAP, BUMPMAP, NORMALMAP,

			GROUP, POSITION, TEXTUREPOS, NORMAL, FACE, PARAMETER, 
		};

		__EBGF_OBJ_CParser()
		{
			File = NULL;
		}

		~__EBGF_OBJ_CParser()
		{
			Close();
		}

		bool FEOF()
		{
			return feof(File);
		}

		void Close()
		{
			if(File) fclose(File);
		}

		bool Open(const char *fname)
		{
			Close();
			if(!(File = fopen(fname, "rt"))) return false;
			RepeatLine = false;
			return true;
		}

		void ReadLine()
		{
			CurString = LineBuffer;
	
			if(RepeatLine)
			{
				RepeatLine = false;
				return;
			}

			char *Ptr = LineBuffer;
			while(1)
			{
				Ptr[0] = fgetc(File);
				if(feof(File)) break;
				if(Ptr[0] == '\n' || Ptr[0] == '\r')
				{
					char t = fgetc(File);
					if(t != '\r' && t != '\n') ungetc(t, File);
						break;
				}
				Ptr++;
			}
			Ptr[0] = '\0';
		}

		void UnReadLine()
		{
			RepeatLine = true;
		}

		void SkipWhiteSpace()
		{
			// ignore initial whitespace
			while((*CurString == ' ' || *CurString == '\t') && *CurString)
				CurString++;
		}

		Commands GetCommand()
		{
			Commands cmd = GetCommandInner();
			while(cmd == COMMENT)
			{
				ReadLine();
				cmd = GetCommandInner();
			}
			return cmd;
		}

		const char *GetString()
		{
			SkipWhiteSpace();

			static char String[2048], *SP;
			strcpy(String, CurString);
			SP = String;
			while(*SP != ' ' && *SP) SP++;
			*SP = '\0';
			CurString += SP - String;
			return String;
		}

		float GetNumber()
		{
			SkipWhiteSpace();
			float R;
			char *End;
			R = strtof(CurString, &End);
			if(CurString == End)
				return -1;
			CurString = End;
			return R;
		}

		void SkipChar()
		{
			CurString++;
		}

		CTexture *GetTexture(const char *file)
		{
			const char *Name = GetString();

			// need to copy directory in here...
			const char *PathName = __EBGF_OBJ_SpliceDirectory(file, Name);

			// and load
			CTexture *T = EBGF_GetTexture(PathName);
			if(!T) printf("Failed to load: %s\n", PathName);
			return T;
		}

	private:
		FILE *File;

		char *CurString;
		char LineBuffer[2048];
		bool RepeatLine;

		Commands GetCommandInner()
		{
			struct Token {char *id; Commands Cmd;} TokenArray[] =
			{
				{"#", COMMENT},
				{"newmtl", NEWMATERIAL},
				{"Ns", SPECULAREXPONENT},
				{"d", DISSOLVE},
				{"Ni", OPTICALDENSITY},
				{"illum", ILLUMINATIONMODEL},
				{"Ka", AMBIENTCOLOUR},
				{"Kd", DIFFUSECOLOUR},
				{"Ks", SPECULARCOLOUR},
				{"map_Ka", AMBIENTMAP},
				{"map_Kd", DIFFUSEMAP},
				{"map_Ks", SPECULARMAP},
				{"map_Bump", BUMPMAP},
				{"map_Normal", NORMALMAP}, // I made this one up
				{"f", FACE},
				{"vt", TEXTUREPOS},
				{"vn", NORMAL},
				{"vp", PARAMETER},
				{"v", POSITION},
				{"usemtl", USEMATERIAL},
				{"mtllib", LOADMATERIALS},
				{"g", GROUP},
				{NULL}
			}, *TP = TokenArray;

			SkipWhiteSpace();

			//check for end of line
			if(!(*CurString)) return ENDOFLINE;

			// check other stuff
			while(TP->id)
			{
				if(!strncmp(CurString, TP->id, strlen(TP->id)))
				{
					CurString += strlen(TP->id);
					return TP->Cmd;
				}
				TP++;
			}

			// unrecognised...
			return UNKNOWN;
		}
};

/*

	EBGF_ReadMaterials opens the file 'file' and loads all materials into memory, storing them for
	later access with the given prefix

*/
bool EBGF_ReadMaterials(const char *prefix, const char *file)
{
	__EBGF_OBJ_CParser MatFile;

	if(!MatFile.Open(file)) return false;

	while(1)
	{
		if(MatFile.FEOF()) break;

		// read a new line of input
		MatFile.ReadLine();

		// check for command on line
		switch(MatFile.GetCommand())
		{
			// default reaction is the same as the reaction to comments - ignore the line
			case __EBGF_OBJ_CParser::ENDOFLINE:
			case __EBGF_OBJ_CParser::UNKNOWN:
			default:
			break;

			case __EBGF_OBJ_CParser::NEWMATERIAL:
			{
				const char *Name;

				Name = MatFile.GetString();

				/* check if this material has already been loaded */
				CMaterial *M;
				if(M = __EBGF_OBJ_FindMaterial(prefix, Name))
				{
					bool Done = false;
					while(!Done && !MatFile.FEOF())
					{
						MatFile.ReadLine();
						switch(MatFile.GetCommand())
						{
							default:
							break;
							case __EBGF_OBJ_CParser::NEWMATERIAL:
								MatFile.UnReadLine();
								Done = true;
							break;
						}
					}
				}
				else
				{
					M = new CMaterial;
					M->RefCount = 0;

					M->Filename = strdup(__EBGF_OBJ_GetMaterialName(prefix, Name));
					__EBGF_StoreHash(M);
					int Maps = 0;

					bool Done = false;
					while(!Done && !MatFile.FEOF())
					{
						MatFile.ReadLine();
						switch(MatFile.GetCommand())
						{
							default:
							case __EBGF_OBJ_CParser::UNKNOWN:
								Done = true;
							break;
							case __EBGF_OBJ_CParser::NEWMATERIAL:
								MatFile.UnReadLine();
								Done = true;
							break;
							case __EBGF_OBJ_CParser::ENDOFLINE:
							break;

							case __EBGF_OBJ_CParser::SPECULAREXPONENT:		M->SpecularExponent = MatFile.GetNumber();			break;
							case __EBGF_OBJ_CParser::DISSOLVE:				M->Dissolve = MatFile.GetNumber();					break;
							case __EBGF_OBJ_CParser::ILLUMINATIONMODEL:		M->IlluminationModel = (int)MatFile.GetNumber();	break;
							case __EBGF_OBJ_CParser::OPTICALDENSITY:		M->OpticalDensity = MatFile.GetNumber();			break;
							case __EBGF_OBJ_CParser::DIFFUSEMAP:			if(M->DiffuseMap = MatFile.GetTexture(file)) Maps |= 1;	break;
							case __EBGF_OBJ_CParser::AMBIENTMAP:			if(M->AmbientMap = MatFile.GetTexture(file)) Maps |= 2;	break;
							case __EBGF_OBJ_CParser::SPECULARMAP:
								EBGF_SetNextTextureFormat(EBGFTC_GREYSCALE);
								if(M->SpecularMap = MatFile.GetTexture(file))	Maps |= 4;
							break;
							case __EBGF_OBJ_CParser::BUMPMAP:
								EBGF_ReturnResource(M->NormalMap); Maps &= ~8;
								EBGF_SetNextTextureFormat(EBGFTC_BUMPTONORMAL);
								if(M->NormalMap = MatFile.GetTexture(file))	Maps |= 8;
							break;
							case __EBGF_OBJ_CParser::NORMALMAP:
								EBGF_ReturnResource(M->NormalMap); Maps &= ~8;
								if(M->NormalMap = MatFile.GetTexture(file))	Maps |= 8;
							break;

							case __EBGF_OBJ_CParser::AMBIENTCOLOUR:
								M->AmbientColour[0] = MatFile.GetNumber();
								M->AmbientColour[1] = MatFile.GetNumber();
								M->AmbientColour[2] = MatFile.GetNumber();
							break;
							case __EBGF_OBJ_CParser::SPECULARCOLOUR:
								M->SpecularColour[0] = MatFile.GetNumber();
								M->SpecularColour[1] = MatFile.GetNumber();
								M->SpecularColour[2] = MatFile.GetNumber();
							break;
							case __EBGF_OBJ_CParser::DIFFUSECOLOUR:
								M->DiffuseColour[0] = MatFile.GetNumber();
								M->DiffuseColour[1] = MatFile.GetNumber();
								M->DiffuseColour[2] = MatFile.GetNumber();
							break;
						}
					}
					switch(Maps)
					{
						default: printf("unhandled: %d\n", Maps);
						case 0: M->ShaderMode = CMaterial::PLAIN;					break;
						case 1: M->ShaderMode = CMaterial::DIFFUSE;					break;
						case 5: M->ShaderMode = CMaterial::DIFFUSE_SPECULAR;		break;
						case 9: M->ShaderMode = CMaterial::DIFFUSE_BUMP;			break;
						case 13: M->ShaderMode = CMaterial::DIFFUSE_SPECULAR_BUMP;	break;
					}
					M->MakeShader();
				}
			}
			break;
		}
	}

	MatFile.Close();
	return true;
}

void CMaterial::Activate(GLfloat *TextureSpaceMatrices)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbientColour);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, DiffuseColour);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecularColour);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, SpecularExponent);

	/* check for shader */
	if(Shader && Shader->Activate())
	{
		switch(ShaderMode)
		{
			default:
			case PLAIN:
				// no textures
			break;

			case DIFFUSE:
				// enable diffuse map on unit 0
				glActiveTextureARB(GL_TEXTURE0_ARB);
				DiffuseMap->Activate();
			break;

			case DIFFUSE_BUMP:
				NormalSlot = glGetUniformLocationARB(Shader->GetProgramHandle(), "NormalTexture");
				DiffuseSlot = glGetUniformLocationARB(Shader->GetProgramHandle(), "DiffuseTexture");
				glUniform1iARB(NormalSlot, 1);
				glUniform1iARB(DiffuseSlot, 0);
				MatrixSlot = glGetAttribLocationARB(Shader->GetProgramHandle(), "TextureSpaceMatrix");
				glVertexAttribPointerARB(MatrixSlot, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, &TextureSpaceMatrices[0]);
				glVertexAttribPointerARB(MatrixSlot+1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, &TextureSpaceMatrices[3]);
				glVertexAttribPointerARB(MatrixSlot+2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, &TextureSpaceMatrices[6]);
				glEnableVertexAttribArrayARB(MatrixSlot);
				glEnableVertexAttribArrayARB(MatrixSlot+1);
				glEnableVertexAttribArrayARB(MatrixSlot+2);

				// enable diffuse map on unit 0, bump on unit 1
				glActiveTextureARB(GL_TEXTURE1_ARB);
				NormalMap->Activate();
				glActiveTextureARB(GL_TEXTURE0_ARB);
				DiffuseMap->Activate();
			break;
		}
	}
	else
	{
		if(DiffuseMap)
		{
			glEnable(GL_TEXTURE_2D);
			DiffuseMap->Activate();
		}
		else
			glDisable(GL_TEXTURE_2D);
	}
}

bool CMaterial::Restore()
{
	switch(ShaderMode)
	{
		case DIFFUSE_BUMP:
//			Shader->Activate();
		break;
	}
	return true;
}

void CMaterial::MakeShader()
{
	if(!(AvailableGLExtensions&GLEXT_SHADEROBJS)) return;

	Shader = new CShaderPair;
	switch(ShaderMode)
	{
		default:
		case PLAIN:
		{
			char *VertexScript = "\
				varying vec3 N, v;\
				void main(void)\
				{\
					v = vec3(gl_ModelViewMatrix * gl_Vertex);\
					N = normalize(gl_NormalMatrix * gl_Normal);\
					gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
				}";
			char *FragmentScript = "\
				varying vec3 N, v;\
				void main(void)\
				{\
					vec3 L = normalize(gl_LightSource[0].position.xyz - v);\
					vec3 E = normalize(-v);\
					vec3 R = normalize(-reflect(L, N));\
					\
					vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N, L), 0.0);\
					vec4 Ispec = gl_FrontLightProduct[0].specular * pow( max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);\
					\
					gl_FragColor = gl_FrontLightModelProduct.sceneColor + Idiff + Ispec;\
				}";
			Shader->CreateFromListing(VertexScript, FragmentScript);
		}
		break;
		case DIFFUSE:
		{
			char *VertexScript = "\
				varying vec3 N, v;\
				void main(void)\
				{\
					v = vec3(gl_ModelViewMatrix * gl_Vertex);\
					N = normalize(gl_NormalMatrix * gl_Normal);\
					gl_TexCoord[0] = gl_MultiTexCoord0;\
					gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
				}";
			char *FragmentScript = "\
				varying vec3 N, v;\
				uniform sampler2D DiffuseTexture;\
				void main(void)\
				{\
					vec3 Dir = gl_LightSource[0].position.xyz - v;\
					float dist = length(Dir);\
					vec3 L = normalize(gl_LightSource[0].position.xyz - v);\
					vec3 E = normalize(-v);\
					vec3 R = normalize(-reflect(L, N));\
					\
					vec4 Idiff = texture2D(DiffuseTexture, vec2(gl_TexCoord[0])) * gl_FrontLightProduct[0].diffuse * max(dot(N, L), 0.0);\
					vec4 Ispec = gl_FrontLightProduct[0].specular * pow( max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);\
					\
					float Attenuation = 1.0 / (	gl_LightSource[0].constantAttenuation +\
												gl_LightSource[0].linearAttenuation * dist +\
												gl_LightSource[0].quadraticAttenuation * dist * dist);\
					\
					gl_FragColor = Attenuation * (gl_FrontLightModelProduct.sceneColor + Idiff + Ispec);\
				}";
			Shader->CreateFromListing(VertexScript, FragmentScript);
		}
		break;
		case DIFFUSE_BUMP:
		{
			char *VertexScript = "\
				varying vec3 v;\
				varying mat3 TSM;\
				attribute mat3 TextureSpaceMatrix;\
				void main(void)\
				{\
					v = vec3(gl_ModelViewMatrix * gl_Vertex);\
					gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
					TSM = gl_NormalMatrix * TextureSpaceMatrix;\
					gl_TexCoord[0] = gl_MultiTexCoord0;\
				}";

			char *FragmentScript = "\
				varying vec3 v;\
				varying mat3 TSM;\
				uniform sampler2D DiffuseTexture;\
				uniform sampler2D NormalTexture;\
				void main(void)\
				{\
					normalize(TSM[0]);\
					normalize(TSM[1]);\
					normalize(TSM[2]);\
				\
					vec3 Dir = gl_LightSource[0].position.xyz - v;\
					float dist = length(Dir);\
					vec3 L = normalize(Dir);\
					vec3 E = normalize(-v);\
					vec3 BumpNormal = vec3(texture2D(NormalTexture, gl_TexCoord[0].xy));\
					BumpNormal = (BumpNormal - 0.5) * 2.0;\
					BumpNormal = TSM * BumpNormal;\
					BumpNormal = normalize(BumpNormal);\
					vec3 R = normalize(-reflect(L, BumpNormal));\
					\
					vec4 Idiff = texture2D(DiffuseTexture, vec2(gl_TexCoord[0])) * gl_FrontLightProduct[0].diffuse * max(dot(BumpNormal, L), 0.0);\
					vec4 Ispec = gl_FrontLightProduct[0].specular * pow( max(dot(R,E), 0.0), 0.3 * gl_FrontMaterial.shininess);\
					\
					float Attenuation = 1.0 / (	gl_LightSource[0].constantAttenuation +\
												gl_LightSource[0].linearAttenuation * dist +\
												gl_LightSource[0].quadraticAttenuation * dist * dist);\
					\
					gl_FragColor = Attenuation * (gl_FrontLightModelProduct.sceneColor + Ispec + Idiff);\
				}";

			Shader->CreateFromListing(VertexScript, FragmentScript);
		}
		break;
	}
	Restore();
}

CMaterial::CMaterial()
{
	AmbientMap = DiffuseMap = SpecularMap = NormalMap = NULL;
	SpecularExponent = 1;
	AmbientColour[0] = AmbientColour[1] = AmbientColour[2] = AmbientColour[3] = 
	SpecularColour[0] = SpecularColour[1] = SpecularColour[2] = SpecularColour[3] = 
	DiffuseColour[0] = DiffuseColour[1] = DiffuseColour[2] = DiffuseColour[3] = 1;
	Shader = NULL;
}

CMaterial::~CMaterial()
{
	EBGF_ReturnResource(AmbientMap);
	EBGF_ReturnResource(DiffuseMap);
	EBGF_ReturnResource(SpecularMap);
	EBGF_ReturnResource(NormalMap);
	delete Shader;
}

CObject::CObject()
{
	Segments = NULL;
	Verts = NULL;
	InitIAList();

	Positions = NULL;
	TexCoords = NULL;
	Normals = NULL;
	ArrayDA = NULL;
	ElementsDA = NULL;
	TextureSpaceMatrices = NULL;
}

CObject::~CObject()
{
	EBGF_ReturnResource(Verts);
	MaterialGroup *MG = Segments;
	while(MG)
	{
		EBGF_ReturnResource(MG->Material);
		EBGF_ReturnResource(MG->Elements);

		MaterialGroup *N = MG->Next;
		delete MG;
		MG = N;
	}

	delete Positions;
	delete TexCoords;
	delete Normals;
	delete ArrayDA;
	delete ElementsDA;
	delete TextureSpaceMatrices;
}

void CObject::InitIAList()
{
	int c = 65536;
	while(c--)
		IAList[c] = NULL;
}

void CObject::ClearIAList()
{
	int c = 65536;
	while(c--)
	{
		while(IAList[c])
		{
			IAPointer *N = IAList[c]->Next;
			delete IAList[c];
			IAList[c] = N;
		}
	}
}

int CObject::GetIA(int VertexNum, int NormalNum, int TextureNum)
{
	int idx = VertexNum&65535;
	IAPointer *P = IAList[idx];
	while(P)
	{
		if(P->VertexNum == VertexNum && P->NormalNum == NormalNum && P->TextureNum == TextureNum)
			return P->ArrayEntry;
		P = P->Next;
	}

	// P not found, create
	IAPointer *N = new IAPointer;
	N->Next = IAList[idx];
	IAList[idx] = N;

	N->VertexNum = VertexNum;
	N->NormalNum = NormalNum;
	N->TextureNum = TextureNum;

	// OBJ indexing starts at 1...
	VertexNum--; NormalNum--; TextureNum--;

	// create entry in DA
	int c = ArrayDA->GetNum();
	ArrayDA->Add(TexCoords->Get((TextureNum*2)+0)); ArrayDA->Add(1-TexCoords->Get((TextureNum*2)+1));
	ArrayDA->Add(Normals->Get(NormalNum*3)); ArrayDA->Add(Normals->Get((NormalNum*3)+1)); ArrayDA->Add(Normals->Get((NormalNum*3)+2));
	ArrayDA->Add(Positions->Get(NormalNum*3)); ArrayDA->Add(Positions->Get((NormalNum*3)+1)); ArrayDA->Add(Positions->Get((NormalNum*3)+2));
	
	// also create entry in texture space matrix array, completely zero'd for now
	TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0);
	TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0);
	TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0); TextureSpaceMatrices->Add(0);

	N->ArrayEntry = c / 8;
	
	return N->ArrayEntry;
}

void CObject::AddTextureSpaceMatrix(int V1, int V2, int V3)
{
	/* want a matrix that maps from texture space to world space. NOT VICE VERSA */

	// read some values back
	CVector Pos1(ArrayDA->Get((V1*8)+5), ArrayDA->Get((V1*8)+6), ArrayDA->Get((V1*8)+7));
	CVector Pos2(ArrayDA->Get((V2*8)+5), ArrayDA->Get((V2*8)+6), ArrayDA->Get((V2*8)+7));
	CVector Pos3(ArrayDA->Get((V3*8)+5), ArrayDA->Get((V3*8)+6), ArrayDA->Get((V3*8)+7));
	CVector Tex1(ArrayDA->Get((V1*8)+0), ArrayDA->Get((V1*8)+1), 0);
	CVector Tex2(ArrayDA->Get((V2*8)+0), ArrayDA->Get((V2*8)+1), 0);
	CVector Tex3(ArrayDA->Get((V3*8)+0), ArrayDA->Get((V3*8)+1), 0);

	// construct first effort Matrix, for reduction to 2d
	CMatrix M;
	CVector Pos3M1 = Pos3 - Pos1;
	CVector Pos2M1 = Pos2 - Pos1;
	CVector Tex3M1 = Tex3 - Tex1;
	CVector Tex2M1 = Tex2 - Tex1;

	CVector Tangent = 
			(Pos2M1*Tex3M1.Data[1] - Pos3M1*Tex2M1.Data[1]) /
			(Tex2M1.Data[0]*Tex3M1.Data[1] - Tex2M1.Data[1]*Tex3M1.Data[0]);

	CVector Binormal = 
			(Pos2M1*Tex3M1.Data[0] - Pos3M1*Tex2M1.Data[0]) /
			(Tex2M1.Data[1]*Tex3M1.Data[0] - Tex2M1.Data[0]*Tex3M1.Data[1]);

	// twiddle lengths & starts
	CVector Normal = Pos2M1 ^ Pos3M1;

	Normal.Normalise(); Tangent.Normalise(); Binormal.Normalise();
	M.Construct(Tangent, Binormal, Normal);
	M.Transpose();

	int Vs[3] = {V1, V2, V3};
	int c = 3;
	while(c--)
	{
		int CV = Vs[c];
		TextureSpaceMatrices->GetArray()[(CV*9)+0] += M.Contents[0];
		TextureSpaceMatrices->GetArray()[(CV*9)+1] += M.Contents[1];
		TextureSpaceMatrices->GetArray()[(CV*9)+2] += M.Contents[2];
		TextureSpaceMatrices->GetArray()[(CV*9)+3] += M.Contents[4];
		TextureSpaceMatrices->GetArray()[(CV*9)+4] += M.Contents[5];
		TextureSpaceMatrices->GetArray()[(CV*9)+5] += M.Contents[6];
		TextureSpaceMatrices->GetArray()[(CV*9)+6] += M.Contents[8];
		TextureSpaceMatrices->GetArray()[(CV*9)+7] += M.Contents[9];
		TextureSpaceMatrices->GetArray()[(CV*9)+8] += M.Contents[10];
	}
}

void CObject::FixLists()
{
	int c = TextureSpaceMatrices->GetNum() / 3;
	while(c--)
	{
		float x, y, z;
		x = TextureSpaceMatrices->GetArray()[(c*3)+0];
		y = TextureSpaceMatrices->GetArray()[(c*3)+1];
		z = TextureSpaceMatrices->GetArray()[(c*3)+2];

		float l = 1.0f / sqrt(x*x + y*y + z*z);
		x *= l; y *= l; z *= l;
		TextureSpaceMatrices->GetArray()[(c*3)+0] = x;
		TextureSpaceMatrices->GetArray()[(c*3)+1] = y;
		TextureSpaceMatrices->GetArray()[(c*3)+2] = z;
	}
}

bool CObject::Open(const char *filename)
{
	__EBGF_OBJ_CParser OBJFile;

	if(!OBJFile.Open(filename)) return false;

	const char *PathName = __EBGF_OBJ_GetDirectory(filename);
	Positions = new DataAdder<GLfloat>;
	TexCoords = new DataAdder<GLfloat>;
	Normals = new DataAdder<GLfloat>;
	ArrayDA = new DataAdder<GLfloat>;
	ElementsDA = new DataAdder<GLuint>;
	TextureSpaceMatrices = new DataAdder<GLfloat>;

	while(1)
	{
		if(OBJFile.FEOF()) break;

		// read a new line of input
		OBJFile.ReadLine();

		// check for command on line
		switch(OBJFile.GetCommand())
		{
			default:
			break;

			case __EBGF_OBJ_CParser::LOADMATERIALS:
			{
				const char *MtlFile = OBJFile.GetString();
				const char *FileName;

				FileName = __EBGF_OBJ_SpliceDirectory(filename, MtlFile);
				EBGF_ReadMaterials(PathName, FileName);
			}
			break;

			case __EBGF_OBJ_CParser::GROUP:
//				printf("group (don't care)\n");
			break;

#define CopyVector(target, length)\
	{\
		int c = length;\
		while(c--)\
		{\
			GLfloat v = OBJFile.GetNumber();\
			target->Add(v);\
		}\
	}

			case __EBGF_OBJ_CParser::POSITION:		CopyVector(Positions, 3);	break;
			case __EBGF_OBJ_CParser::NORMAL:		CopyVector(Normals, 3);		break;
			case __EBGF_OBJ_CParser::TEXTUREPOS:	CopyVector(TexCoords, 2);	break;

#undef CopyVector

			case __EBGF_OBJ_CParser::USEMATERIAL:
			{
				MaterialGroup *N = new MaterialGroup;
				N->Next = Segments;
				Segments = N;

				const char *MaterialName = OBJFile.GetString();
				N->Material = EBGF_GetMaterial(PathName, MaterialName);

				bool Done = false;
				while(!Done && !OBJFile.FEOF())
				{
					OBJFile.ReadLine();
					switch(OBJFile.GetCommand())
					{
						default:
							OBJFile.UnReadLine();
							Done = true;
						break;
						case __EBGF_OBJ_CParser::FACE:
						{
							int Ptrs[3];
							for(int c = 0; c < 3; c++)
							{
								// read a number/number/number combo
								int VertNum = OBJFile.GetNumber();		OBJFile.SkipChar();
								int TextureNum = OBJFile.GetNumber();	OBJFile.SkipChar();
								int NormalNum = OBJFile.GetNumber();

								if(VertNum == -1 || NormalNum == -1)
									break;

								ElementsDA->Add(Ptrs[c] = GetIA(VertNum, NormalNum, TextureNum));
							}
							AddTextureSpaceMatrix(Ptrs[0], Ptrs[1], Ptrs[2]);
						}
						break;
					}
				}
				FixLists();
				N->Elements = new CDrawElements(GL_TRIANGLES, ElementsDA->GetNum(), GL_UNSIGNED_INT, ElementsDA->GetArray());
				ElementsDA->Flush();
			}
			break;
		}
	}

//	printf("%d v %d\n", ArrayDA->GetNum() / 8, TextureSpaceMatrices->GetNum() / 9);
	Verts = new CInterleavedArrays(GL_T2F_N3F_V3F, 0, ArrayDA->GetArray(), ArrayDA->GetNum() / 8);

	delete ArrayDA; ArrayDA = NULL;
	delete Positions; Positions = NULL;
	delete TexCoords; TexCoords = NULL;
	delete Normals; Normals = NULL;
	delete ElementsDA; ElementsDA = NULL;

	OBJFile.Close();
	return true;
}

void CObject::Draw()
{
	if(Verts) Verts->Enable();

	MaterialGroup *MG = Segments;
	while(MG)
	{
		if(MG->Elements)
		{
			if(MG->Material) 
				MG->Material->Activate(TextureSpaceMatrices->GetArray());
//		printf("%d\n", TextureSpaceMatrices->GetNum());
//		printf("%d\n", MG->Elements->GetNum());
			MG->Elements->Draw();
		}
		MG = MG->Next;
	}
}

template <class T>
void CObject::DataAdder<T>::Add(T v)
{
	if(NumAllocated == NumStored)
	{
		NumAllocated += 256;
		Array = (T *)realloc(Array, sizeof(T)*NumAllocated);
	}
	Array[NumStored++] = v;
}

template <class T>
void CObject::DataAdder<T>::Flush()
{
	free(Array); Array = NULL;
	NumAllocated = NumStored = 0;
}
