#include "Freescape.h"
#include <stdio.h>

CFreescapeGame::CObject::CObject(CFreescapeGame *P)
{
	Parent = P;
	Condition = NULL;
	GroupSize = 0;
}

CFreescapeGame::CObject::~CObject()
{
	delete Condition;
	Condition = NULL;
}

void CFreescapeGame::CObject::Move(float x, float y, float z)
{
	if(Moveable || Type == GROUP)
	{
		if(Type == GROUP)
		{
			int c = GroupSize;
			while(c--)
			{
				CObject *O = Parent->GetObject(ObjectPtrs[c]);
				if(O) O->Move(x, y, z);
			}
		}
		Pos[0] += x; Pos[1] += y; Pos[2] += z;
//		printf("+%0.2f %0.2f %0.2f -> %0.2f %0.2f %0.2f\n", x, y, z, Pos[0], Pos[1], Pos[2]);
	}
}

void CFreescapeGame::CObject::MoveTo(float x, float y, float z)
{
	if(Moveable || Type == GROUP)
	{
		if(Type == GROUP)
		{
			x -= Pos[0]; y -= Pos[1]; z -= Pos[2];
			int c = GroupSize;
			while(c--)
			{
				CObject *O = Parent->GetObject(ObjectPtrs[c]);
				if(O) O->Move(x, y, z);
			}
			Pos[0] += x; Pos[1] += y; Pos[2] += z;
		}
		else
		{
			Pos[0] = x; Pos[1] = y; Pos[2] = z;
		}
	}
}

void CFreescapeGame::CObject::Reset()
{
	if(Moveable)
	{
		Pos[0] = StartPos[0]; Pos[1] = StartPos[1]; Pos[2] = StartPos[2];
	}
	CurState = DefaultState;
	if(Condition)
	{
		Condition->SetLooping(false);
		Condition->SetActive(false);
		Condition->Reset();
	}
	Shot = Activated = Collided = false;
}

bool CFreescapeGame::CObject::GetCollided()		{ return Collided; }
bool CFreescapeGame::CObject::GetShot()			{ return Shot; }
bool CFreescapeGame::CObject::GetActivated()	{ return Activated; }
void CFreescapeGame::CObject::SetCollided(bool t)	{if(Collided = t) if(Condition) Condition->SetActive(true);}
void CFreescapeGame::CObject::SetShot(bool t)		{if(Shot = t) if(Condition) Condition->SetActive(true);}
void CFreescapeGame::CObject::SetActivated(bool t)	{if(Activated = t) if(Condition) Condition->SetActive(true);}

void CFreescapeGame::CObject::SetCondition(CFreescapeGame::CCondition *C)
{
	delete Condition;
	Condition = C;
}

CFreescapeGame::CCondition *CFreescapeGame::CObject::GetCondition()
{
	return Condition;
}

bool CFreescapeGame::CObject::GetVis()
{
	return CurState == VISIBLE;
}

void CFreescapeGame::CObject::SetVis(bool v)
{
	if(CurState != DESTROYED)
	{
		CurState = v ? VISIBLE : INVISIBLE;
		if(Type == GROUP)
		{
			int c = GroupSize;
			while(c--)
			{
				CObject *O = Parent->GetObject(ObjectPtrs[c]);
				if(O) O->SetVis(v);
			}
		}
	}
}

void CFreescapeGame::CObject::Destroy()
{
	CurState = DESTROYED;
	if(Type == GROUP)
	{
		int c = GroupSize;
		while(c--)
		{
			CObject *O = Parent->GetObject(ObjectPtrs[c]);
			if(O) O->Destroy();
		}
	}
}

void CFreescapeGame::CObject::SetDefaultVisible(bool v)
{
	DefaultState = v ? VISIBLE : INVISIBLE;
}

float *CFreescapeGame::CObject::GetPos()
{
	return Pos;
}

float *CFreescapeGame::CObject::GetSize()
{
	return Size;
}

void CFreescapeGame::CObject::SetType(ObjectTypes T)
{
	Type = T;
}

CFreescapeGame::ObjectTypes CFreescapeGame::CObject::GetType()
{
	return Type;
}

void CFreescapeGame::CObject::SetMoveable(bool t)
{
	Moveable = t;
}

bool CFreescapeGame::CObject::GetMoveable()
{
	return Moveable;
}

void CFreescapeGame::CObject::SetLocation(float *P, float *S)
{
	Pos[0] = P[0]; Pos[1] = P[1]; Pos[2] = P[2];
	Size[0] = S[0]; Size[1] = S[1]; Size[2] = S[2];
}

void CFreescapeGame::CObject::SetStartPos(float *P)
{
	StartPos[0] = Pos[0]; StartPos[1] = Pos[1]; StartPos[2] = Pos[2];
}

void CFreescapeGame::CObject::SetNumSides(int c)
{
	NumSides = c;
}

int CFreescapeGame::CObject::GetNumSides()
{
	switch(Type)
	{
		case PYRAMID: return 6;
		case RECTANGLE: return 2;
		case CUBOID: return 6;
		case SENSOR:
		case GROUP:
		return 0;
	}
	return NumSides;
}

void CFreescapeGame::CObject::SetVertex(int id, float *Pos)
{
	Verts[id][0] = Pos[0];
	Verts[id][1] = Pos[1];
	Verts[id][2] = Pos[2];
}

void CFreescapeGame::CObject::SetPyramidType(int c)
{
	PyramidType = c;
}

void CFreescapeGame::CObject::SetApex(float *A, float *B)
{
	ApexStart[0] = A[0]; ApexStart[1] = A[1];
	ApexEnd[0] = B[0]; ApexEnd[1] = B[1];
}

void CFreescapeGame::CObject::SetColour(unsigned int c, CColour D)
{
	Colours[c] = D;
}

void CFreescapeGame::CObject::SetSensorStats(int S, int R, int DF, int SID)
{
	Speed = S; Range = R; DirFlags = DF; SensorID = SID;
}

void CFreescapeGame::CObject::AddObject(int o)
{
	ObjectPtrs[GroupSize] = o; GroupSize++;
}

bool CFreescapeGame::CObject::CollFace::SetNormal(float *Vert1, float *Vert2, float *Vert3)
{
	CVector V1, V2, V3, E1, E2;
	V1 = Vert1; V2 = Vert2; V3 = Vert3;
	E1 = V2 - V1;
	E2 = V3 - V1;
	
	if(!E1.SQLength()) return false;
	if(!E2.SQLength()) return false;
	
	SitsOn.Normal = E2^E1;
	SitsOn.Normal.Normalise();
	SitsOn.Distance = SitsOn.Normal*V1;

	NumSurrounders = 0;
	return true;
}

void CFreescapeGame::CObject::CollFace::AddEdge(float *Vert1, float *Vert2)
{
	CVector V1, V2, E;
	V1 = Vert1;
	V2 = Vert2;
	E = V2 - V1;

	if(!E.SQLength()) return;

	Surrounders[NumSurrounders].Normal = E^SitsOn.Normal; 
	Surrounders[NumSurrounders].Normal.Normalise(); 
	Surrounders[NumSurrounders].Distance = V1*Surrounders[NumSurrounders].Normal;
	NumSurrounders++;
}

void CFreescapeGame::CObject::Assemble()
{
	NumFaces = 0;
	switch(Type)
	{
		case GROUP:
		{
			if(GroupSize)
			{
				CObject *O = Parent->GetObject(ObjectPtrs[0]);
				Pos[0] = O->Pos[0]; Pos[1] = O->Pos[1]; Pos[2] = O->Pos[2];
			}
			
			int c = GroupSize;
			while(c--)
			{
				CObject *O = Parent->GetObject(ObjectPtrs[c]);
				if(O->Pos[0] < Pos[0]) Pos[0] = O->Pos[0];
				if(O->Pos[1] < Pos[1]) Pos[1] = O->Pos[1];
				if(O->Pos[2] < Pos[2]) Pos[2] = O->Pos[2];
			}
		}
		break;
		case PLANAR:
		{
			if(NumSides > 2)
			{
				NumFaces = 2;
				Faces[0].SetNormal(Verts[0], Verts[1], Verts[2]);
				Faces[1].SetNormal(Verts[2], Verts[1], Verts[0]);

				int c = NumSides;
				while(c--)
				{
					Faces[0].AddEdge(Verts[c], Verts[(c+1)%NumSides]);
					Faces[1].AddEdge(Verts[(c+1)%NumSides], Verts[c]);
				}

				if(!Size[0])
				{
					c =NumSides;
					while(c--)
					{
						Verts[c][0]++;
					}
					Pos[0] -= 1;
					Size[0] += 2;
				}
				if(!Size[1])
				{
					c =NumSides;
					while(c--)
					{
						Verts[c][1]++;
					}
					Pos[1] -= 1;
					Size[1] += 2;
				}
				if(!Size[2])
				{
					c =NumSides;
					while(c--)
					{
						Verts[c][2]++;
					}
					Pos[2] -= 1;
					Size[2] += 2;
				}
			}
		}
		break;

#define LSet(n, x, y, z) LVerts[n][0] = x; LVerts[n][1] = y; LVerts[n][2] = z;
#define UploadQuad()\
			Faces[NumFaces].SetNormal(LVerts[0], LVerts[1], LVerts[2]);\
			c = 4;\
			while(c--)\
				Faces[NumFaces].AddEdge(LVerts[c], LVerts[(c+1)%4]);\
			NumFaces++;
#define UploadQuadA(A, B, C, D)\
			if(!Faces[NumFaces].SetNormal(A, B, C))\
				if(!Faces[NumFaces].SetNormal(A, C, D))\
					Faces[NumFaces].SetNormal(A, B, D);\
			Faces[NumFaces].AddEdge(A, B);\
			Faces[NumFaces].AddEdge(B, C);\
			Faces[NumFaces].AddEdge(C, D);\
			Faces[NumFaces].AddEdge(D, A);\
			NumFaces++;

		case CUBOID:
		case RECTANGLE:
		{
			float LVerts[4][3];
			NumFaces = 0;
			int c;

			if(Size[0] && Size[1])
			{
				LSet(0, 0, 0, 0); LSet(1, Size[0], 0, 0); LSet(2, Size[0], Size[1], 0); LSet(3, 0, Size[1], 0);
				UploadQuad();
				LSet(0, Size[0], 0, Size[2]); LSet(1, 0, 0, Size[2]); LSet(2, 0, Size[1], Size[2]); LSet(3, Size[0], Size[1], Size[2]);
				UploadQuad();
			}
			if(Size[2] && Size[1])
			{
				LSet(0, 0, 0, Size[2]); LSet(1, 0, 0, 0); LSet(2, 0, Size[1], 0); LSet(3, 0, Size[1], Size[2]);
				UploadQuad();
				LSet(0, Size[0], 0, 0); LSet(1, Size[0], 0, Size[2]); LSet(2, Size[0], Size[1], Size[2]); LSet(3, Size[0], Size[1], 0);
				UploadQuad();
			}
			if(Size[0] && Size[2])
			{
				LSet(0, 0, 0, 0); LSet(1, 0, 0, Size[2]); LSet(2, Size[0], 0, Size[2]); LSet(3, Size[0], 0, 0);
				UploadQuad();
				LSet(0, 0, Size[1], Size[2]); LSet(1, 0, Size[1], 0); LSet(2, Size[0], Size[1], 0); LSet(3, Size[0], Size[1], Size[2]);
				UploadQuad();
			}
		}
		break;

		case PYRAMID:
		{
			float Verts[8][3];
			NumFaces = 0;
#define Set(n, x, y, z) Verts[n][0] = x; Verts[n][1] = y; Verts[n][2] = z;
				switch(PyramidType)
				{
/*					default:
						printf("don't know how to draw %d\n", PyramidType);
						goto end;
					break;*/
					case 3: //
						Set(0, 0,		0,	0);
						Set(1, Size[0],	0,	0);
						Set(2, Size[0],	0,	Size[2]);
						Set(3, 0,		0,	Size[2]);

						Set(4, ApexStart[0],	Size[1],	ApexStart[1]);
						Set(5, ApexEnd[0],		Size[1],	ApexStart[1]);
						Set(6, ApexEnd[0],		Size[1],	ApexEnd[1]);
						Set(7, ApexStart[0],	Size[1],	ApexEnd[1]);
					break;
					case 4: //
						Set(5, ApexStart[0], 0, ApexStart[1]);
						Set(4, ApexEnd[0], 0, ApexStart[1]);
						Set(7, ApexEnd[0], 0, ApexEnd[1]);
						Set(6, ApexStart[0], 0, ApexEnd[1]);

						Set(1, 0, Size[1], 0);
						Set(0, Size[0], Size[1], 0);
						Set(3, Size[0], Size[1], Size[2]);
						Set(2, 0, Size[1], Size[2]);
					break;
					case 1:
						Set(3, 0,	0,			0);
						Set(2, 0,	Size[1],	0);
						Set(1, 0,	Size[1],	Size[2]);
						Set(0, 0,	0,			Size[2]);

						Set(7, Size[0],	ApexStart[0],	ApexStart[1]);
						Set(6, Size[0],	ApexEnd[0],	ApexStart[1]);
						Set(5, Size[0],	ApexEnd[0],	ApexEnd[1]);
						Set(4, Size[0],	ApexStart[0],	ApexEnd[1]);
					break;
					case 2: // perfect per area 4
						Set(0, Size[0],	0,			0);
						Set(1, Size[0],	Size[1],	0);
						Set(2, Size[0],	Size[1],	Size[2]);
						Set(3, Size[0],	0,			Size[2]);

						Set(4, 0,	ApexStart[0],	ApexStart[1]);
						Set(5, 0,	ApexEnd[0],	ApexStart[1]);
						Set(6, 0,	ApexEnd[0],	ApexEnd[1]);
						Set(7, 0,	ApexStart[0],	ApexEnd[1]);
					break;
					case 6: //
						Set(0, 0,		0,			Size[2]);
						Set(1, Size[0],	0,			Size[2]);
						Set(2, Size[0],	Size[1],	Size[2]);
						Set(3, 0,		Size[1],	Size[2]);

						Set(4, ApexStart[0],	ApexStart[1],	0);
						Set(5, ApexEnd[0],		ApexStart[1],	0);
						Set(6, ApexEnd[0],		ApexEnd[1],	0);
						Set(7, ApexStart[0],	ApexEnd[1],	0);
					break;
					case 5: // perfect
						Set(3, 0,		0,			0);
						Set(2, Size[0],	0,			0);
						Set(1, Size[0],	Size[1],	0);
						Set(0, 0,		Size[1],	0);

						Set(7, ApexStart[0],	ApexStart[1],	Size[2]);
						Set(6, ApexEnd[0],		ApexStart[1],	Size[2]);
						Set(5, ApexEnd[0],		ApexEnd[1],	Size[2]);
						Set(4, ApexStart[0],	ApexEnd[1],	Size[2]);
					break;
				}
#undef Set

				UploadQuadA(Verts[1], Verts[2], Verts[6], Verts[5]);
				UploadQuadA(Verts[3], Verts[0], Verts[4], Verts[7]);
				UploadQuadA(Verts[0], Verts[1], Verts[5], Verts[4]);
				UploadQuadA(Verts[2], Verts[3], Verts[7], Verts[6]);
				UploadQuadA(Verts[3], Verts[2], Verts[1], Verts[0]);
				UploadQuadA(Verts[4], Verts[5], Verts[6], Verts[7]);
		}
		break;
	}
}
#undef LSet
#undef UploadQuad

void CFreescapeGame::CObject::Draw()
{
	if(CurState != VISIBLE) return;
//	if(Shot) {Shot = false; return;}

	glPushMatrix();
		glTranslatef(Pos[0], Pos[1], Pos[2]);
				
	switch(Type)
	{
		case RECTANGLE:
			glPolygonOffset(-1, -1);
			glBegin(GL_QUADS);
				if(!Size[0])
				{
					if(Colours[1].Entry)
					{
						glColor3fv(Colours[1].Col);
						glVertex3f(0, 0, 0);
						glVertex3f(0, 0, Size[2]);
						glVertex3f(0, Size[1], Size[2]);
						glVertex3f(0, Size[1], 0);
					}

					if(Colours[0].Entry)
					{
						glColor3fv(Colours[0].Col);
						glVertex3f(0, 0, Size[2]);
						glVertex3f(0, 0, 0);
						glVertex3f(0, Size[1], 0);
						glVertex3f(0, Size[1], Size[2]);
					}
				}
				if(!Size[1])
				{
					if(Colours[1].Entry)
					{
						glColor3fv(Colours[1].Col);
						glVertex3f(0, 0, Size[2]);
						glVertex3f(0, 0, 0);
						glVertex3f(Size[0], 0, 0);
						glVertex3f(Size[0], 0, Size[2]);
					}

					if(Colours[0].Entry)
					{
						glColor3fv(Colours[0].Col);
						glVertex3f(0, 0, 0);
						glVertex3f(0, 0, Size[2]);
						glVertex3f(Size[0], 0, Size[2]);
						glVertex3f(Size[0], 0, 0);
					}
				}
				if(!Size[2])
				{
					if(Colours[0].Entry)
					{
						glColor3fv(Colours[0].Col);
						glVertex3f(0, 0, 0);
						glVertex3f(Size[0], 0, 0);
						glVertex3f(Size[0], Size[1], 0);
						glVertex3f(0, Size[1], 0);
					}

					if(Colours[1].Entry)
					{
						glColor3fv(Colours[1].Col);
						glVertex3f(Size[0], 0, 0);
						glVertex3f(0, 0, 0);
						glVertex3f(0, Size[1], 0);
						glVertex3f(Size[0], Size[1], 0);
					}
				}
			glEnd();
		break;
		case PLANAR:
			glPolygonOffset(-1, -1);
				if(NumSides > 2)
				{
					if(Colours[0].Entry)
					{
						glBegin(GL_POLYGON);
						glColor3fv(Colours[0].Col);
						for(int c = 0; c < NumSides; c++)
							glVertex3f(Verts[c][0], Verts[c][1], Verts[c][2]);
						glEnd();
					}
					if(Colours[1].Entry)
					{
						glBegin(GL_POLYGON);
						glColor3fv(Colours[1].Col);
						int c = NumSides;
						while(c--)
							glVertex3f(Verts[c][0], Verts[c][1], Verts[c][2]);
						glEnd();
					}
				}
				else
				{
					glColor3fv(Colours[0].Col);
					glBegin(GL_LINES);
						glVertex3f(Verts[0][0], Verts[0][1], Verts[0][2]);
						glVertex3f(Verts[1][0], Verts[1][1], Verts[1][2]);
					glEnd();
				}
		break;
		case CUBOID:
			glPolygonOffset(0, 0);
			glBegin(GL_QUADS);
				if(Colours[4].Entry)
				{
					glColor3fv(Colours[4].Col);
					glVertex3f(0, 0, 0);
					glVertex3f(Size[0], 0, 0);
					glVertex3f(Size[0], Size[1], 0);
					glVertex3f(0, Size[1], 0);
				}

				if(Colours[5].Entry)
				{
					glColor3fv(Colours[5].Col);
					glVertex3f(Size[0], 0, Size[2]);
					glVertex3f(0, 0, Size[2]);
					glVertex3f(0, Size[1], Size[2]);
					glVertex3f(Size[0], Size[1], Size[2]);
				}

				if(Colours[0].Entry)
				{
					glColor3fv(Colours[0].Col);
					glVertex3f(0, 0, Size[2]);
					glVertex3f(0, 0, 0);
					glVertex3f(0, Size[1], 0);
					glVertex3f(0, Size[1], Size[2]);
				}

				if(Colours[1].Entry)
				{
					glColor3fv(Colours[1].Col);
					glVertex3f(Size[0], 0, 0);
					glVertex3f(Size[0], 0, Size[2]);
					glVertex3f(Size[0], Size[1], Size[2]);
					glVertex3f(Size[0], Size[1], 0);
				}

				if(Colours[2].Entry)
				{
					glColor3fv(Colours[2].Col);
					glVertex3f(0, 0, 0);
					glVertex3f(0, 0, Size[2]);
					glVertex3f(Size[0], 0, Size[2]);
					glVertex3f(Size[0], 0, 0);
				}

				if(Colours[3].Entry)
				{
					glColor3fv(Colours[3].Col);
					glVertex3f(0, Size[1], Size[2]);
					glVertex3f(0, Size[1], 0);
					glVertex3f(Size[0], Size[1], 0);
					glVertex3f(Size[0], Size[1], Size[2]);
				}

			glEnd();
		break;
		case PYRAMID:
		{
//			bool D = true;
			GLfloat Verts[8][3];
			glPolygonOffset(0, 0);
				
#define Set(n, x, y, z) Verts[n][0] = x; Verts[n][1] = y; Verts[n][2] = z;
				switch(PyramidType)
				{
					default:
						printf("don't know how to draw %d\n", PyramidType);
						goto end;
					break;
					case 3: //
						Set(0, 0,		0,	0);
						Set(1, Size[0],	0,	0);
						Set(2, Size[0],	0,	Size[2]);
						Set(3, 0,		0,	Size[2]);

						Set(4, ApexStart[0],	Size[1],	ApexStart[1]);
						Set(5, ApexEnd[0],		Size[1],	ApexStart[1]);
						Set(6, ApexEnd[0],		Size[1],	ApexEnd[1]);
						Set(7, ApexStart[0],	Size[1],	ApexEnd[1]);
					break;
					case 4: //
						Set(5, ApexStart[0], 0, ApexStart[1]);
						Set(4, ApexEnd[0], 0, ApexStart[1]);
						Set(7, ApexEnd[0], 0, ApexEnd[1]);
						Set(6, ApexStart[0], 0, ApexEnd[1]);

						Set(1, 0, Size[1], 0);
						Set(0, Size[0], Size[1], 0);
						Set(3, Size[0], Size[1], Size[2]);
						Set(2, 0, Size[1], Size[2]);
					break;
					case 1:
						Set(3, 0,	0,			0);
						Set(2, 0,	Size[1],	0);
						Set(1, 0,	Size[1],	Size[2]);
						Set(0, 0,	0,			Size[2]);

						Set(7, Size[0],	ApexStart[0],	ApexStart[1]);
						Set(6, Size[0],	ApexEnd[0],	ApexStart[1]);
						Set(5, Size[0],	ApexEnd[0],	ApexEnd[1]);
						Set(4, Size[0],	ApexStart[0],	ApexEnd[1]);
					break;
					case 2: // perfect per area 4
						Set(0, Size[0],	0,			0);
						Set(1, Size[0],	Size[1],	0);
						Set(2, Size[0],	Size[1],	Size[2]);
						Set(3, Size[0],	0,			Size[2]);

						Set(4, 0,	ApexStart[0],	ApexStart[1]);
						Set(5, 0,	ApexEnd[0],	ApexStart[1]);
						Set(6, 0,	ApexEnd[0],	ApexEnd[1]);
						Set(7, 0,	ApexStart[0],	ApexEnd[1]);
					break;
					case 6: //
						Set(0, 0,		0,			Size[2]);
						Set(1, Size[0],	0,			Size[2]);
						Set(2, Size[0],	Size[1],	Size[2]);
						Set(3, 0,		Size[1],	Size[2]);

						Set(4, ApexStart[0],	ApexStart[1],	0);
						Set(5, ApexEnd[0],		ApexStart[1],	0);
						Set(6, ApexEnd[0],		ApexEnd[1],	0);
						Set(7, ApexStart[0],	ApexEnd[1],	0);
					break;
					case 5: // perfect
						Set(3, 0,		0,			0);
						Set(2, Size[0],	0,			0);
						Set(1, Size[0],	Size[1],	0);
						Set(0, 0,		Size[1],	0);

						Set(7, ApexStart[0],	ApexStart[1],	Size[2]);
						Set(6, ApexEnd[0],		ApexStart[1],	Size[2]);
						Set(5, ApexEnd[0],		ApexEnd[1],	Size[2]);
						Set(4, ApexStart[0],	ApexEnd[1],	Size[2]);
					break;
				}
#undef Set
			glBegin(GL_QUADS);

				if(Colours[0].Entry)
				{
					glColor3fv(Colours[0].Col);
					glVertex3fv(Verts[1]);
					glVertex3fv(Verts[2]);
					glVertex3fv(Verts[6]);
					glVertex3fv(Verts[5]);
				}

				if(Colours[1].Entry)
				{
					glColor3fv(Colours[1].Col);
					glVertex3fv(Verts[3]);
					glVertex3fv(Verts[0]);
					glVertex3fv(Verts[4]);
					glVertex3fv(Verts[7]);
				}
	
				if(Colours[2].Entry)
				{
					glColor3fv(Colours[2].Col);
					glVertex3fv(Verts[0]);
					glVertex3fv(Verts[1]);
					glVertex3fv(Verts[5]);
					glVertex3fv(Verts[4]);
				}

				if(Colours[3].Entry)
				{
					glColor3fv(Colours[3].Col);
					glVertex3fv(Verts[2]);
					glVertex3fv(Verts[3]);
					glVertex3fv(Verts[7]);
					glVertex3fv(Verts[6]);
				}

				if(Colours[4].Entry)
				{
					glColor3fv(Colours[4].Col);
					glVertex3fv(Verts[3]);
					glVertex3fv(Verts[2]);
					glVertex3fv(Verts[1]);
					glVertex3fv(Verts[0]);
				}

				if(Colours[5].Entry)
				{
					glColor3fv(Colours[5].Col);
					glVertex3fv(Verts[4]);
					glVertex3fv(Verts[5]);
					glVertex3fv(Verts[6]);
					glVertex3fv(Verts[7]);
				}

			glEnd();
			end:;
		}
		break;
	}
	glPopMatrix();
}

float CFreescapeGame::CObject::TestRayCollision(float *Start, float *Direction)
{
	if(CurState != VISIBLE) return -1;

	float RelStart[3] = {Start[0] - Pos[0], Start[1] - Pos[1], Start[2] - Pos[2]};
	CVector RS;
	CVector Dir;
	Dir = Direction;

	RS = RelStart;

	int Hit = -1;
	float Distance = -1;

	int c = NumFaces;
	while(c--)
	{
		float d1 = RS*Faces[c].SitsOn.Normal - Faces[c].SitsOn.Distance;
		float d2 = Dir*Faces[c].SitsOn.Normal;

		if(d1 > 0 && d2 < 0)
		{
			float r = -(d1 / d2);
			CVector HitSpot = RS + Dir*r;

			int ic = Faces[c].NumSurrounders;
			while(ic--)
				if((HitSpot*Faces[c].Surrounders[ic].Normal - Faces[c].Surrounders[ic].Distance) < 0) break;

			if(ic < 0)
			{
				if(Hit == -1 || r < Distance)
				{
					Distance = r + (((Type == PLANAR) || (Type == RECTANGLE)) ? -10 : 0);
					Hit = c;
				}
			}
		}
	}
	return Distance;
}

void CFreescapeGame::CObject::Update(float Scale)
{
	if(Type == SENSOR && CurState == VISIBLE)
	{
		float PlayerPos[3] = {Parent->GetVariable(0), Parent->GetVariable(1), Parent->GetVariable(2)};
		float Diffs[3] = {PlayerPos[0] - Pos[0], PlayerPos[1] - Pos[1], PlayerPos[2] - Pos[2]};
		float SQDist = Diffs[0]*Diffs[0] + Diffs[1]*Diffs[1] + Diffs[2]*Diffs[2];
		if(SQDist*Scale < Range*Range)
		{
			bool Seen = false;
			if(Diffs[0] >= 0 && (DirFlags&SENSORFLAG_EAST)) Seen = true;
			if(Diffs[0] <= 0 && (DirFlags&SENSORFLAG_WEST)) Seen = true;
			if(Diffs[1] >= 0 && (DirFlags&SENSORFLAG_UP)) Seen = true;
			if(Diffs[1] <= 0 && (DirFlags&SENSORFLAG_DOWN)) Seen = true;
			if(Diffs[2] >= 0 && (DirFlags&SENSORFLAG_NORTH)) Seen = true;
			if(Diffs[2] <= 0 && (DirFlags&SENSORFLAG_SOUTH)) Seen = true;
/*			if(fabs(Diffs[0]) > fabs(Diffs[1]) && fabs(Diffs[0]) > fabs(Diffs[1]))
			{
				if((Diffs[0] > 0) && (DirFlags&SENSORFLAG_EAST)) Seen = true;
				if((Diffs[0] < 0) && (DirFlags&SENSORFLAG_WEST)) Seen = true;
			}
			if(fabs(Diffs[1]) > fabs(Diffs[0]) && fabs(Diffs[1]) > fabs(Diffs[2]))
			{
				if((Diffs[1] > 0) && (DirFlags&SENSORFLAG_UP)) Seen = true;
				if((Diffs[1] < 0) && (DirFlags&SENSORFLAG_DOWN)) Seen = true;
			}
			if(fabs(Diffs[2]) > fabs(Diffs[0]) && fabs(Diffs[2]) > fabs(Diffs[1]))
			{
				if((Diffs[2] > 0) && (DirFlags&SENSORFLAG_NORTH)) Seen = true;
				if((Diffs[2] < 0) && (DirFlags&SENSORFLAG_SOUTH)) Seen = true;
			}*/

			if(Seen)
			{
//				printf("spotted by %d (%0.2f %0.2f %0.2f v %0.2f %0.2f %0.2f: %0.2f)\n", SensorID, PlayerPos[0], PlayerPos[1], PlayerPos[2], Pos[0], Pos[1], Pos[2], sqrt(SQDist));
				Parent->SetVariable(13, SensorID);
				Parent->SetVariable(14, Parent->GetVariable(14)+1);
			}
		}
	}
}

#define ERROR_BOUND	0.1
bool CFreescapeGame::CObject::CheckIntercept(float *Start, float *Move, float I, float Radius, float Descent, float Ascent)
{
	float IPos[3] = {Start[0] + I*Move[0], Start[1] + I*Move[1], Start[2] + I*Move[2]};

	if((IPos[0]+Radius) < (Pos[0]-ERROR_BOUND)) return false;
	if((IPos[0]-Radius) > (Pos[0]+Size[0]+ERROR_BOUND)) return false;

	if((IPos[1]+Ascent) < (Pos[1]-ERROR_BOUND)) return false;
	if((IPos[1]+Descent) > (Pos[1]+Size[1]+ERROR_BOUND)) return false;

	if((IPos[2]+Radius) < (Pos[2]-ERROR_BOUND)) return false;
	if((IPos[2]-Radius) > (Pos[2]+Size[2]+ERROR_BOUND)) return false;

	return true;
}

float CFreescapeGame::CObject::GetCollisionTime(float *Start, float *Move, float Radius, float Ascent, float Descent)
{
	// if a cylinder starts at 'Start' and moves along 'Move' with a radius of Radius and ascend/descend of Ascend/Descend, when does it hit this object, if at all?

	// simple checks - y bounds
	if(Start[1]+Ascent <= Pos[1] && Start[1]+Move[1]+Ascent <= Pos[1]) return -100;
	if(Start[1]+Descent >= Pos[1]+Size[1] && Start[1]+Move[1]+Descent >= Pos[1]+Size[1]) return -100;

	// x and z bounds
	if(Start[0]+Radius <= Pos[0] && Start[0]+Move[0]+Radius <= Pos[0]) return -100;
	if(Start[0]-Radius >= Pos[0]+Size[0] && Start[0]+Move[0]-Radius >= Pos[0]+Size[0]) return -100;
	if(Start[2]+Radius <= Pos[2] && Start[2]+Move[2]+Radius <= Pos[2]) return -100;
	if(Start[2]-Radius >= Pos[2]+Size[2] && Start[2]+Move[2]-Radius >= Pos[2]+Size[2]) return -100;

	// right then. Definitely a possible hit
	float HitTime = 2;
	float Intercept;
	Supporting = false;

	// check if this object acts as a roof
	if(Start[1]+Ascent <= Pos[1] && Move[1] > 0)
	{
		Intercept = (Pos[1] - (Start[1] + Ascent)) / Move[1];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[0] = HitNormal[2] = 0;
				HitNormal[1] = -1;
			}
	}

	// check if this object acts as a floor
	if(Start[1]+Descent >= Pos[1]+Size[1] && Move[1] < 0)
	{
		Intercept = ((Pos[1]+Size[1]) - (Start[1] + Descent)) / Move[1];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[0] = HitNormal[2] = 0;
				HitNormal[1] = 1;
				Supporting = true;
			}
	}

	// check x sides
	if((Start[0]+Radius) <= (Pos[0]+ERROR_BOUND) && Move[0] > 0)
	{
		Intercept = (Pos[0] - (Start[0]+Radius)) / Move[0];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[0] = -1;
				HitNormal[2] = HitNormal[1] = 0;
				Supporting = false;
			}
	}

	if((Start[0]-Radius) >= (Pos[0]+Size[0]-ERROR_BOUND) && Move[0] < 0)
	{
		Intercept = (Pos[0]+Size[0] - (Start[0]-Radius)) / Move[0];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[0] = 1;
				HitNormal[2] = HitNormal[1] = 0;
				Supporting = false;
			}
	}

	// check y sides
	if(Start[2]+Radius <= Pos[2] && Move[2] > 0)
	{
		Intercept = (Pos[2] - (Start[2]+Radius)) / Move[2];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[2] = -1;
				HitNormal[0] = HitNormal[1] = 0;
				Supporting = false;
			}
	}

	if(Start[2]-Radius >= Pos[2]+Size[2] && Move[2] < 0)
	{
		Intercept = ((Pos[2]+Size[2]) - (Start[2]-Radius)) / Move[2];
		if(CheckIntercept(Start, Move, Intercept, Radius, Descent, Ascent))
			if(Intercept < HitTime)
			{
				HitTime = Intercept;
				HitNormal[2] = 1;
				HitNormal[0] = HitNormal[1] = 0;
				Supporting = false;
			}
	}

	//

	return HitTime < 1 ? HitTime : -100;
}

bool CFreescapeGame::CObject::FixLastCollision(float *Move)
{
	float d = Move[0]*HitNormal[0] + Move[1]*HitNormal[1] + Move[2]*HitNormal[2];
	Move[0] -= d*HitNormal[0];
	Move[1] -= d*HitNormal[1];
	Move[2] -= d*HitNormal[2];

	return Supporting;
}

bool CFreescapeGame::CObject::GetElevation(float *PPos, float Radius, float &El)
{
	if((PPos[0]-(0.8*Radius)) > (Pos[0]+Size[0])) return false;
	if((PPos[0]+(0.8*Radius)) < (Pos[0])) return false;
	if((PPos[2]-(0.8*Radius)) > (Pos[2]+Size[2])) return false;
	if((PPos[2]+(0.8*Radius)) < (Pos[2])) return false;
	El = PPos[1] - (Pos[1] + Size[1]);
	return true;
}

bool CFreescapeGame::CObject::CheckInside(float *PPos)
{
	if(PPos[0] < Pos[0]) return false;
	if(PPos[1] < Pos[1]) return false;
	if(PPos[2] < Pos[2]) return false;
	if(PPos[0] > Pos[0]+Size[0]) return false;
	if(PPos[1] > Pos[1]+Size[1]) return false;
	if(PPos[2] > Pos[2]+Size[2]) return false;
	return true;
}
