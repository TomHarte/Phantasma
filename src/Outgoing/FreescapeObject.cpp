#include "Freescape.h"
#include "ebgf.h"
#include "ebgf/ebgf_DataCollector.h"
#include <stdio.h>

CVector CFreescapeGame::CObject::TempVArray[6];
int CFreescapeGame::CObject::NumVerts;
CFreescapeGame::CColour CFreescapeGame::CObject::CurCol;
DataCollector<GLfloat> *CFreescapeGame::CObject::VertexCollector;
DataCollector<GLuint> *CFreescapeGame::CObject::IndexCollector;

#define SHOT_NUM_FRAMES	10

CFreescapeGame::CObject::CObject(CFreescapeGame *P)
{
	Parent = P;
	Condition = NULL;
	GroupSize = 0;
	DefaultState = VISIBLE;
	PlayerShot = 0;
	NumFaces = 0;
	VArray = NULL;
	Elements = NULL;
	PolyOffset = 0;
}

CFreescapeGame::CObject::~CObject()
{
	delete Condition;
	Condition = NULL;
	delete VArray;
	VArray = NULL;
	delete Elements;
	Elements = NULL;
}

#define MAX_MOVE	256

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
		if(fabs(x) > MAX_MOVE || fabs(y) > MAX_MOVE || fabs(z) > MAX_MOVE)
		{
			Pos[0] += x; Pos[1] += y; Pos[2] += z;
		}
		else
		{
			PosAdd[0] = x * 0.25f; PosAdd[1] = y * 0.25f; PosAdd[2] = z * 0.25f;
			AddSteps = 4;
		}
	}
}

void CFreescapeGame::CObject::MoveTo(float x, float y, float z)
{
	Move(x - Pos[0], y - Pos[1], z - Pos[2]);
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
	AddSteps = 0;
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
//			printf("group setvis...\n");
			int c = GroupSize;
			while(c--)
			{
				Parent->SetVis(ObjectPtrs[c], Area, v);
//				CObject *O = Parent->GetObject(ObjectPtrs[c]);
//				if(O) O->SetVis(v);
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

void CFreescapeGame::CObject::SetNumSides(unsigned int c)
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

void CFreescapeGame::CObject::SetVertex(unsigned int id, float *Pos)
{
	Verts[id][0] = Pos[0];
	Verts[id][1] = Pos[1];
	Verts[id][2] = Pos[2];
}

void CFreescapeGame::CObject::SetPyramidType(unsigned int c)
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

void CFreescapeGame::CObject::AddObject(unsigned int o)
{
	ObjectPtrs[GroupSize] = o; GroupSize++;
}

bool CFreescapeGame::CObject::CollFace::SetNormal(CVector &N, float D)
{
	SitsOn.Normal = N;
	SitsOn.Distance = D;

	NumSurrounders = 0;
	return true;
}

void CFreescapeGame::CObject::CollFace::AddEdge(CVector &V1, CVector &V2)
{
	CVector E;
	E = V2 - V1;

	if(!E.SQLength()) return;

	Surrounders[NumSurrounders].Normal = E^SitsOn.Normal; 
	Surrounders[NumSurrounders].Normal.Normalise(); 
	Surrounders[NumSurrounders].Distance = V1*Surrounders[NumSurrounders].Normal;
	NumSurrounders++;
}

void CFreescapeGame::CObject::BeginFace(CColour col)
{
	CurCol = col;
	NumVerts = 0;
}

void CFreescapeGame::CObject::AddVert(float *V)
{
	TempVArray[NumVerts] = V;
	// kill same spot vertices here
	if(NumVerts > 0)
	{
		CVector V1 = TempVArray[NumVerts];
		CVector V2 = TempVArray[NumVerts-1];
		CVector Edge = V1 - V2;
		if(Edge.SQLength() < 0.001) return;
	}
	NumVerts++;
}

void CFreescapeGame::CObject::AddVert(float x, float y, float z)
{
	float Array[3] = {x, y, z};
	AddVert(Array);
}

void CFreescapeGame::CObject::EndFace()
{
	if(NumVerts < 3 || !CurCol.Entry) return;

	CVector E1, E2;
	E1 = TempVArray[1] - TempVArray[0];
	E2 = TempVArray[2] - TempVArray[0];

	CVector Normal = E2^E1;
	Normal.Normalise();
	float Distance = Normal*TempVArray[0];

	Faces[NumFaces].SetNormal(Normal, Distance);
	int c = NumVerts;
	while(c--)
		Faces[NumFaces].AddEdge(TempVArray[c], TempVArray[(c+1)%NumVerts]);

	NumFaces++;

	/* and add to vertex / face array - aiming for GL_C4F_N3F_V3F */
	int IndexBase = VertexCollector->GetNum() / 10;
	for(c = 0; c < NumVerts; c++)
	{
		VertexCollector->Add(CurCol.Col[0]); VertexCollector->Add(CurCol.Col[1]); VertexCollector->Add(CurCol.Col[2]); VertexCollector->Add(1);
		VertexCollector->Add(Normal.Data[0]); VertexCollector->Add(Normal.Data[1]); VertexCollector->Add(Normal.Data[2]);
		VertexCollector->Add(TempVArray[c].Data[0]); VertexCollector->Add(TempVArray[c].Data[1]); VertexCollector->Add(TempVArray[c].Data[2]);
	}
	for(c = 0; c < NumVerts-2; c++)
	{
		IndexCollector->Add(IndexBase);
		IndexCollector->Add(IndexBase+1+c);
		IndexCollector->Add(IndexBase+2+c);
	}
}

void CFreescapeGame::CObject::Assemble()
{
	VertexCollector = new DataCollector<GLfloat>;
	IndexCollector = new DataCollector<GLuint>;

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
			PolyOffset = 1;
			if(NumSides > 2)
			{
				BeginFace(Colours[0]);
					for(int c = 0; c < NumSides; c++)
						AddVert(Verts[c]);
				EndFace();
				BeginFace(Colours[1]);
					int c = NumSides;
					while(c--)
						AddVert(Verts[c]);
				EndFace();
			}
			else
			{
				GLfloat LocVerts[] = {
									Colours[0].Col[0], Colours[0].Col[1], Colours[0].Col[2], Verts[0][0], Verts[0][1], Verts[0][2],
									Colours[0].Col[0], Colours[0].Col[1], Colours[0].Col[2], Verts[1][0], Verts[1][1], Verts[1][2]};
				GLubyte Indices[] = {0, 1};
				VArray = new CInterleavedArrays(GL_C3F_V3F, 0, LocVerts, 2);
				Elements = new CDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, Indices);
			}
		}
		break;
		case RECTANGLE:
		{
			PolyOffset = 1;
			if(!Size[0])
			{
				BeginFace(Colours[0]);
					AddVert(0, 0, Size[2]); AddVert(0, 0, 0); AddVert(0, Size[1], 0); AddVert(0, Size[1], Size[2]); 
				EndFace();
				BeginFace(Colours[1]);
					AddVert(0, 0, 0); AddVert(0, 0, Size[2]); AddVert(0, Size[1], Size[2]); AddVert(0, Size[1], 0);
				EndFace();
			}
			if(!Size[1])
			{
				BeginFace(Colours[0]);
					AddVert(0, 0, 0); AddVert(0, 0, Size[2]); AddVert(Size[0], 0, Size[2]); AddVert(Size[0], 0, 0); 
				EndFace();
				BeginFace(Colours[1]);
					AddVert(0, 0, Size[2]); AddVert(0, 0, 0); AddVert(Size[0], 0, 0); AddVert(Size[0], 0, Size[2]);
				EndFace();
			}
			if(!Size[2])
			{
				BeginFace(Colours[0]);
					AddVert(0, 0, 0); AddVert(Size[0], 0, 0); AddVert(Size[0], Size[1], 0); AddVert(0, Size[1], 0);
				EndFace();
				BeginFace(Colours[1]);
					AddVert(Size[0], 0, 0); AddVert(0, 0, 0); AddVert(0, Size[1], 0); AddVert(Size[0], Size[1], 0); 
				EndFace();
			}
		}
		break;
		case CUBOID:
			BeginFace(Colours[4]);
				AddVert(0, 0, 0); AddVert(Size[0], 0, 0); AddVert(Size[0], Size[1], 0); AddVert(0, Size[1], 0);
			EndFace();
			BeginFace(Colours[5]);
				AddVert(Size[0], 0, Size[2]); AddVert(0, 0, Size[2]); AddVert(0, Size[1], Size[2]); AddVert(Size[0], Size[1], Size[2]);
			EndFace();
			BeginFace(Colours[0]);
				AddVert(0, 0, Size[2]); AddVert(0, 0, 0); AddVert(0, Size[1], 0); AddVert(0, Size[1], Size[2]);
			EndFace();
			BeginFace(Colours[1]);
				AddVert(Size[0], 0, 0); AddVert(Size[0], 0, Size[2]); AddVert(Size[0], Size[1], Size[2]); AddVert(Size[0], Size[1], 0);
			EndFace();
			BeginFace(Colours[2]);
				AddVert(0, 0, 0); AddVert(0, 0, Size[2]); AddVert(Size[0], 0, Size[2]); AddVert(Size[0], 0, 0);
			EndFace();
			BeginFace(Colours[3]);
				AddVert(0, Size[1], Size[2]); AddVert(0, Size[1], 0); AddVert(Size[0], Size[1], 0); AddVert(Size[0], Size[1], Size[2]);
			EndFace();
		break;
		case PYRAMID:
		{
			GLfloat Verts[8][3];
#define Set(n, x, y, z) Verts[n][0] = x; Verts[n][1] = y; Verts[n][2] = z;
				switch(PyramidType)
				{
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
			BeginFace(Colours[0]);
				AddVert(Verts[1]); AddVert(Verts[2]); AddVert(Verts[6]); AddVert(Verts[5]);
			EndFace();
			BeginFace(Colours[1]);
				AddVert(Verts[3]); AddVert(Verts[0]); AddVert(Verts[4]); AddVert(Verts[7]);
			EndFace();
			BeginFace(Colours[2]);
				AddVert(Verts[0]); AddVert(Verts[1]); AddVert(Verts[5]); AddVert(Verts[4]);
			EndFace();
			BeginFace(Colours[3]);
				AddVert(Verts[2]); AddVert(Verts[3]); AddVert(Verts[7]); AddVert(Verts[6]);
			EndFace();
			BeginFace(Colours[4]);
				AddVert(Verts[3]); AddVert(Verts[2]); AddVert(Verts[1]); AddVert(Verts[0]);
			EndFace();
			BeginFace(Colours[5]);
				AddVert(Verts[4]); AddVert(Verts[5]); AddVert(Verts[6]); AddVert(Verts[7]);
			EndFace();
		}
		break;
	}

	// make display object
	if(VertexCollector->GetNum())
	{
		VArray = new CInterleavedArrays(GL_C4F_N3F_V3F, 0, VertexCollector->GetArray(), VertexCollector->GetNum() / 10);
		Elements = new CDrawElements(GL_TRIANGLES, IndexCollector->GetNum(), GL_UNSIGNED_INT, IndexCollector->GetArray());
	}
	delete VertexCollector;
	delete IndexCollector;
}

void CFreescapeGame::CObject::Draw(float *PlayerPos)
{
	if(CurState != VISIBLE) return;

	if(PlayerShot)
	{
		if((Speed - PlayerShot) < SHOT_NUM_FRAMES)
		{
			Parent->AddShot(Pos);
		}
	}

	glPushMatrix();
		glTranslatef(Pos[0], Pos[1], Pos[2]);
		if(VArray && Elements)
		{
			glPolygonOffset(-PolyOffset, -PolyOffset);
			VArray->Enable();
			Elements->Draw();
			VArray->Disable();
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

void CFreescapeGame::CObject::Update()
{
	if(PlayerShot)
		PlayerShot--;
	if(AddSteps)
	{
		AddSteps--;
		Pos[0] += PosAdd[0]; Pos[1] += PosAdd[1]; Pos[2] += PosAdd[2];
	}
}

void CFreescapeGame::CObject::UpdateLogic(float *PlayerPos, float Scale)
{
	if(Type == SENSOR && CurState == VISIBLE)
	{
		float Diffs[3] = {Pos[0] - PlayerPos[0], Pos[1] - PlayerPos[1], Pos[2] - PlayerPos[2]};
		float Dist = fabs(Diffs[0]) + fabs(Diffs[1]) + fabs(Diffs[2]); // it's Manhattan distance!
		if(Dist < Range)
		{

			bool Seen = false;
			if((Diffs[0] > 0) && (DirFlags&SENSORFLAG_EAST)) Seen = true;
			if((Diffs[0] < 0) && (DirFlags&SENSORFLAG_WEST)) Seen = true;
			if((Diffs[1] > 0) && (DirFlags&SENSORFLAG_UP)) Seen = true;
			if((Diffs[1] < 0) && (DirFlags&SENSORFLAG_DOWN)) Seen = true;
			if((Diffs[2] > 0) && (DirFlags&SENSORFLAG_NORTH)) Seen = true;
			if((Diffs[2] < 0) && (DirFlags&SENSORFLAG_SOUTH)) Seen = true;

			if(Seen)
			{
				CArea *P = Parent->GetCurArea();
				float Dir[3] = {PlayerPos[0] - Pos[0], PlayerPos[1] - Pos[1], PlayerPos[2] - Pos[2]};
				if(P->FindFrontObject(Pos, Dir) >= 0)
				{
					float d = P->GetFrontDistance();
					if(d < 0.99f)
						Seen = false;
				}
			}

			if(Seen)
			{
				Parent->SetVariable(13, SensorID);
				Parent->SetVariable(14, Parent->GetVariable(14)+1);
				if(!PlayerShot)
				{
					Parent->PlaySound(1);
					Parent->SetVariable(11, Parent->GetVariable(11)+1);
					PlayerShot = Speed;
				}
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
	// only for objects that have moved or changed visibility!
	return false;
	if(PPos[0] < Pos[0]) return false;
	if(PPos[1] < Pos[1]) return false;
	if(PPos[2] < Pos[2]) return false;
	if(PPos[0] > Pos[0]+Size[0]) return false;
	if(PPos[1] > Pos[1]+Size[1]) return false;
	if(PPos[2] > Pos[2]+Size[2]) return false;
	return true;
}

void CFreescapeGame::CObject::SetArea(unsigned int a) {Area = a;}
