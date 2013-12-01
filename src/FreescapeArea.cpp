#include "Freescape.h"
#include <stdio.h>
#include <math.h>

CFreescapeGame::CArea::CArea(CFreescapeGame *P)
{
	Parent = P;
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		Objects[c] = NULL;
	c = MAX_CONDITIONS_PER_AREA;
	while(c--)
		Conditions[c] = NULL;
	c = MAX_ANIMATORS_PER_AREA;
	while(c--)
		Animators[c] = NULL;
	c = MAX_ENTRANCES_PER_AREA;
	while(c--)
		Entrances[c] = NULL;

	Scale = 1;
	HasFloor = true;
}

CFreescapeGame::CArea::~CArea()
{
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
	{
		delete Objects[c];
		Objects[c] = NULL;
	}
	c = MAX_CONDITIONS_PER_AREA;
	while(c--)
	{
		delete Conditions[c];
		Conditions[c] = NULL;
	}
	c = MAX_ANIMATORS_PER_AREA;
	while(c--)
	{
		delete Animators[c];
		Animators[c] = NULL;
	}
	c = MAX_ENTRANCES_PER_AREA;
	while(c--)
	{
		delete Entrances[c];
		Entrances[c] = NULL;
	}
}

void CFreescapeGame::CArea::AddObject(unsigned int id, CObject *Obj)
{
	if(id >= MAX_OBJECTS_PER_AREA) return;
	if(Objects[id]) delete Objects[id];
	Objects[id] = Obj;
}

CFreescapeGame::CObject *CFreescapeGame::CArea::GetObject(unsigned int id)
{
	if(id >= MAX_OBJECTS_PER_AREA) return NULL;
	return Objects[id];
}

void CFreescapeGame::CArea::AddCondition(unsigned int id, CCondition *Condition)
{
	if(id >= MAX_CONDITIONS_PER_AREA) return;
	if(Conditions[id]) delete Conditions[id];
	Conditions[id] = Condition;
}

CFreescapeGame::CCondition *CFreescapeGame::CArea::GetCondition(unsigned int id)
{
	if(id >= MAX_CONDITIONS_PER_AREA) return NULL;
	return Conditions[id];
}

void CFreescapeGame::CArea::AddAnimator(unsigned int id, CCondition *Animator)
{
	if(id >= MAX_ANIMATORS_PER_AREA) return;
	if(Animators[id]) delete Animators[id];
	Animators[id] = Animator;
}

CFreescapeGame::CCondition *CFreescapeGame::CArea::GetAnimator(unsigned int id)
{
	if(id >= MAX_ANIMATORS_PER_AREA) return NULL;
	return Animators[id];
}

float CFreescapeGame::CArea::GetScale()
{
	return Scale;
}

void CFreescapeGame::CArea::SetScale(float s)
{
	Scale = s;
}

void CFreescapeGame::CArea::LoadScale()
{
	float ScaleFactor = (float)Scale / ((float)256.0f);
	glScalef(-ScaleFactor, ScaleFactor, ScaleFactor);
}

void CFreescapeGame::CArea::SetupDisplay()
{
	glClearColor(Sky.Col[0], Sky.Col[1], Sky.Col[2], 1);
}

void CFreescapeGame::CArea::Reset()
{
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		if(Objects[c])
			Objects[c]->Reset();
	c = MAX_CONDITIONS_PER_AREA;
	while(c--)
		if(Conditions[c])
		{
			Conditions[c]->SetLooping(true);
			Conditions[c]->SetActive(true);
			Conditions[c]->Reset();
		}
	c = MAX_ANIMATORS_PER_AREA;
	while(c--)
		if(Animators[c])
		{
			Animators[c]->SetLooping(false);
			Animators[c]->SetActive(false);
			Animators[c]->Reset();
		}
}

void CFreescapeGame::CArea::Draw()
{
	if(HasFloor)
	{
		glColor3fv(Floor.Col);
		glBegin(GL_QUADS);
		glVertex3f(-100000, 0, -100000);
		glVertex3f(100000, 0, -100000);
		glVertex3f(100000, 0, 100000);
		glVertex3f(-100000, 0, 100000);
		glEnd();
	}
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		if(Objects[c])
			Objects[c]->Draw();
}

void CFreescapeGame::CArea::Update()
{
	static int Framec = 0;
	Framec++;

	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		if(Objects[c])
		{
			CCondition *C = Objects[c]->GetCondition();
			if(C)
			{
				if(!C->Execute(Objects[c]))
					printf("failed on object %d\n", c);
			}
			Objects[c]->Update(Scale);
			if(Parent->StopProcessing()) return;
		}

	c = MAX_CONDITIONS_PER_AREA;
	while(c--)
		if(Conditions[c])
		{
			Conditions[c]->Execute(NULL);
			if(Parent->StopProcessing()) return;
		}

	c = MAX_ANIMATORS_PER_AREA;
	while(c--)
		if(Animators[c])
		{
			Animators[c]->Execute(NULL);
			if(Parent->StopProcessing()) return;
		}
}

void CFreescapeGame::CArea::SetEntrance(unsigned int id, Entrance *E)
{
	if(id >= MAX_ENTRANCES_PER_AREA) return;
	delete Entrances[id];
	Entrances[id] = E;
}

CFreescapeGame::CArea::Entrance *CFreescapeGame::CArea::GetEntrance(unsigned int id)
{
	if(id >= MAX_ENTRANCES_PER_AREA) return NULL;
	return Entrances[id];
}

#define PLAYER_WIDTH	32.0
#define ERROR_BOUND		0.001

bool CFreescapeGame::CArea::FixCollisions(float *Pos, float *Move, float Radius, float Ascent, float Descent)
{
	bool Supported = false;

	// proper collision stuff
	while((Move[0]*Move[0] + Move[1]*Move[1] + Move[2]*Move[2]) >= 0.001)
	{
		float HitTime;
		CObject *HitObj = NULL;
		CObject *BackupObjs[8]; int BackupPtr = 0;
		int c = MAX_OBJECTS_PER_AREA;
		while(c--)
			if(Objects[c] && Objects[c]->GetVis() && Objects[c]->GetType() != GROUP)
			{
				float HT = Objects[c]->GetCollisionTime(Pos, Move, Radius, Ascent, Descent);
				if(HT >= -10)
				{
					if(fabs(HT - HitTime) < ERROR_BOUND && HitObj)
					{
						if(HT < HitTime)
							HitTime = HT;
						if(BackupPtr < 8)
							BackupObjs[BackupPtr++] = Objects[c];
					}
					else
						if(HT < HitTime || !HitObj)
						{
							HitObj = Objects[c];
							HitTime = HT;
							BackupPtr = 0;
						}
				}
			}

		if(HitObj)
		{
			Pos[0] += HitTime*Move[0]; 
			Pos[1] += HitTime*Move[1]; 
			Pos[2] += HitTime*Move[2];
			if(HitTime > 0)
			{
				Move[0] *= (1.0f - HitTime);
				Move[1] *= (1.0f - HitTime);
				Move[2] *= (1.0f - HitTime);
			}
			Supported |= HitObj->FixLastCollision(Move);
			HitObj->SetCollided(true);
			while(BackupPtr--)
				BackupObjs[BackupPtr]->SetCollided(true);
		}
		else
		{
			Pos[0] += Move[0]; Pos[1] += Move[1]; Pos[2] += Move[2];
			break;
		}
	}

	// push up step
	int c = MAX_OBJECTS_PER_AREA;
	CObject *PushObj = NULL;
	float PushL = 0;
	while(c--)
		if(Objects[c] && Objects[c]->GetVis() && Objects[c]->GetType() != GROUP)
		{
			float L;
			if(Objects[c]->GetElevation(Pos, Radius, L))
			{
				if(L < 0 && L >= -(Descent+1) || Objects[c]->CheckInside(Pos))
				{
					Pos[1] -= L;//*0.25f;
					Supported = true;
					if(L < PushL)
					{
						PushL = L;
						PushObj = Objects[c];
					}
				}
			}
		}

	if(PushObj) PushObj->SetCollided(true);

	// and boundaries must be observed
/*	if(Pos[0] < 0) Pos[0] = 0;
	if(Pos[2] < 0) Pos[2] = 0;
	if(Pos[0] > 4080) Pos[0] = 4080;
	if(Pos[2] > 4080) Pos[2] = 4080;*/
//	printf("%0.2f %0.2f\n", Pos[0], Pos[2]);

	// all areas have an infinite floor, so...
	if(Pos[1] < 0)
	{
		Pos[1] = 0;
		return true;
	}
	
	return Supported;
}

void CFreescapeGame::CArea::SetColours(CFreescapeGame::CColour &Fl, CFreescapeGame::CColour &Sk)
{
	Floor = Fl; Sky = Sk;
}

int CFreescapeGame::CArea::FindFrontObject(float *Pos, float *Vec)
{
	int Hit = -1;
	float HitPos = 0;
	DistanceToHit = -1;
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
	{
		if(Objects[c])
		{
			float NewHit = Objects[c]->TestRayCollision(Pos, Vec);
			if(NewHit >= 0)
			{
				if(NewHit < HitPos || Hit == -1)
				{
					DistanceToHit = HitPos = NewHit;
					Hit = c;
				}
			}
		}
	}
	return Hit;
}

float CFreescapeGame::CArea::GetFrontDistance()
{
	return DistanceToHit;
}

void CFreescapeGame::CArea::Assemble()
{
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		if(Objects[c])
			Objects[c]->Assemble();
}
