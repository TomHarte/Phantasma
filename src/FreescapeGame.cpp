#include "Freescape.h"
#include "ebgf/ebgf.h"
#include "ebgf/ebgf_VertexArray.h"
#include "ebgf/ebgf_DrawElements.h"
#include <stdio.h>
#include <math.h>

#define FORCEDMOVE_PAUSE	10
#define SHOT_PAUSER			15
#define SHOT_LASTDRAW		10
#define MESSAGE_LIFE		1500
#define MESSAGE_SHORTLIFE	150

#define PLAYER_WIDTH	32.0
#define PLAYER_ASCENT	16.0
#define PLAYER_DESCENT	2.0

#define GetRadius()	(PLAYER_WIDTH / Areas[CurArea]->GetScale())
#define GetAscent() ((CurHeight*Areas[CurArea]->GetScale()) + PLAYER_ASCENT)
#define GetDescent() (MaxClimb * Areas[CurArea]->GetScale())

CFreescapeGame::CFreescapeGame()
{
	int c = MAX_AREAS;
	while(c--)
		Areas[c] = NULL;
	CurArea = 0;
	GlobalArea = new CArea(this);
	MoveVec[0] = MoveVec[1] = MoveVec[2] = 0;
	PrintFont = NULL;
	Handler = NULL;

	c = NUM_SOUNDS;
	while(c--)
		Sounds[c] = NULL;
}

CFreescapeGame::~CFreescapeGame()
{
	int c = MAX_AREAS;
	while(c--)
	{
		delete Areas[c];
		Areas[c] = NULL;
	}
	delete GlobalArea;
	EBGF_ReturnResource(PrintFont); PrintFont = NULL;

	c = NUM_SOUNDS;
	while(c--)
		if(Sounds[c])
		{
			Mix_FreeChunk(Sounds[c]);
			Sounds[c] = NULL;
		}

}

/*

	variable gateway. Dumb for now
	
*/
bool CFreescapeGame::GetBit(unsigned int id)
{
	if(id >= NUM_BITS) return false;
	return Bits[id];
}

Sint32 CFreescapeGame::GetVariable(unsigned int id)
{
//	if(id < 22) printf("reserved variable %d checked\n", id);
	if(id >= NUM_VARIABLES) return 0;
	switch(id)
	{
		default:	return Variables[id]&VarMask;
		case 16:	if(ZXVars) return Variables[16]&VarMask; else return (Variables[16] + (ShotLastFrame ? 1 : 0))&VarMask;
	}
}

void CFreescapeGame::SetBit(unsigned int id, bool value)
{
	if(id >= NUM_BITS) return;
	Bits[id] = value;
}

void CFreescapeGame::SetVariable(unsigned int id, Sint32 value)
{
//	if(id < 22) printf("reserved variable %d changed\n", id);
//	if(id < 3) ForcedMoveCounter = FORCEDMOVE_PAUSE;


	if(ZXVars && (id >= 112) && (id <= 120))
	{
		PosDirty = true;
	}

	if(!ZXVars && (id < 6))
	{
		PosDirty = true;
		if(id < 3) ForcedMoveCounter = FORCEDMOVE_PAUSE;
	}

	if(id >= NUM_VARIABLES) return;
	Variables[id] = value&VarMask;
}

int CFreescapeGame::GetCurrentArea()
{
	return CurArea;
}

bool CFreescapeGame::GetVis(unsigned int Object, unsigned int Area)
{
	if(Area >= MAX_AREAS) return false;
	if(!Areas[Area]) return false;
	CObject *O = Areas[Area]->GetObject(Object);
	if(!O) {printf("illegal object %d in area %d\n", Object, Area); return false;}
	return O->GetVis();
}

void CFreescapeGame::SetVis(unsigned int Object, unsigned int Area, bool Visible)
{
	if(Area >= MAX_AREAS) return;
	if(!Areas[Area]) return;
	CObject *O = Areas[Area]->GetObject(Object);
	if(O) O->SetVis(Visible);
}

void CFreescapeGame::Destroy(unsigned int Object, unsigned int Area)
{
	if(Area >= MAX_AREAS) return;
	if(!Areas[Area]) return;
	CObject *O = Areas[Area]->GetObject(Object);
	if(O) O->Destroy();
}

bool CFreescapeGame::QueryTimerTicked()
{
	bool Res = false;
	if(TimerVar >= EBGF_GetLogicFrequency() * Timer)
	{
		Res = true;
		TimerVar = 0;
	}
	return Res;
}

CFreescapeGame::CObject *CFreescapeGame::GetObject(unsigned int id)
{
	return Areas[CurArea]->GetObject(id);
}

void CFreescapeGame::Goto(unsigned int Entrance, unsigned int Area)
{
	printf("goto %d, %d\n", Entrance, Area);

/*	if(Entrance == 3 && Area == 2)
	{
		printf("yo ho ho [%d]\n", (int)Variables[10]);
	}*/

	ProcessingMask = true;

	if(Area >= MAX_AREAS) return;
	if(!Areas[Area]) return;
	const CArea::Entrance *E = Areas[Area]->GetEntrance(Entrance);
	if(!E) return;

	CurArea = Area;
	if(ZXVars)
	{
		Variables[124] = Area;
	}
	else
	{
		Variables[9] = Variables[8];
		Variables[8] = Area;
	}
	PlayerPos[0] = E->Pos[0]; PlayerPos[1] = E->Pos[1]; PlayerPos[2] = E->Pos[2];
	PlayerAng[0] = E->Rotation[0]; PlayerAng[1] = E->Rotation[1]; PlayerAng[2] = E->Rotation[2];
	AddSteps = 0;
	Areas[CurArea]->SetupDisplay();
	Areas[CurArea]->Enter();
	Areas[CurArea]->PushOut(PlayerPos, GetRadius(), GetAscent(), GetDescent());
	LastHeight = PlayerPos[1];

	/* flag all current messages to disappear soon */
	int c = 0;
	while(c < MessagePointer)
	{
		if(Messages[c].Age < MESSAGE_LIFE-MESSAGE_SHORTLIFE) 
			Messages[c].Age = MESSAGE_LIFE-MESSAGE_SHORTLIFE;
		c++;
	}
}

void CFreescapeGame::SetAnimatorActive(unsigned int Animator, unsigned int Area, bool Active)
{
	if(Area >= MAX_AREAS) return;
	if(!Areas[Area]) return;
	CCondition *A = Areas[Area]->GetAnimator(Animator);
	if(A) A->SetActive(Active);
}

void CFreescapeGame::TriggerAnimator(unsigned int Animator)
{
	CCondition *A = Areas[CurArea]->GetAnimator(Animator);
	if(A) A->Trigger();
}

/*
	gameplay!
*/

float CFreescapeGame::FindFrontObject(int &Area, int &Object)
{
	float Angles[3] = {(-PlayerAng[0] / 180) * M_PI, ((PlayerAng[1]) / 180) * M_PI, (PlayerAng[2] / 180) * M_PI};
	float Front[3] = {sin(Angles[1])*cos(Angles[0]), sin(Angles[0]), cos(Angles[1])*cos(Angles[0])};
	float Pos[3] = {PlayerPos[0], PlayerPos[1] + CurHeight*Areas[CurArea]->GetScale(), PlayerPos[2]};

	Area = -1;
	Object = Areas[CurArea]->FindFrontObject(Pos, Front);
	if(Object >= 0) Area = CurArea;
	return Areas[CurArea]->GetFrontDistance();
}

CFreescapeGame::CArea *CFreescapeGame::GetCurArea()
{
	return Areas[CurArea];
}

void CFreescapeGame::Activate()
{
	if(!ZXVars)
		Variables[16] += 2;

	int A, O;
	double Distance;
	Distance = FindFrontObject(A, O);
	if(A >= 0 && Distance < MaxActivateDistance*Areas[CurArea]->GetScale())
	{
		CObject *Obj = Areas[A]->GetObject(O);
		if(Obj)
		{
			Obj->SetActivated(true);
		}
	}
}

void CFreescapeGame::Shoot()
{
	if(ZXVars)
	{
		if(Variables[125] != 255)
		{
			if(!Variables[125]) return;
			Variables[125]--;
		}
	}
	else
		Variables[21]++;

	PlaySound(0);
	ShotLastFrame = SHOT_PAUSER;
	int A, O;
	FindFrontObject(A, O);
	if(A >= 0)
	{
//		printf("%d\n", O);
		CObject *Obj = Areas[A]->GetObject(O);
		if(Obj)	Obj->SetShot(true);
	}
}

void CFreescapeGame::Move(float x, float y, float z, float xr, float yr, float zr)
{
	float Scale = Areas[CurArea]->GetScale();
	x *= Scale;
	y *= Scale;
	z *= Scale;

	float YAng = (PlayerAng[1] * M_PI) / 180.0f;
	float XAng = ((90+PlayerAng[0]) * M_PI) / 180.0f;
	MoveVec[0] = z*sin(YAng);
	MoveVec[2] = z*cos(YAng);

	MoveVec[2] += -x*sin(YAng);
	MoveVec[0] += x*cos(YAng);

	if(PlayMode != 1)	//flying?
	{
		MoveVec[0] *= sin(XAng);
		MoveVec[2] *= sin(XAng);
		MoveVec[1] = z*cos(XAng);
	}

	PlayerAng[0] += xr / 10; PlayerAng[1] += yr / 10; PlayerAng[2] += zr / 10;
}

void CFreescapeGame::SetupDisplay()
{
	if(Areas[CurArea]) Areas[CurArea]->SetupDisplay();
}


#define MAX_MOVE		256
//#define MAX_ROTATE		5

void CFreescapeGame::Update()
{
	float PPos[3] = {PlayerPos[0], PlayerPos[1] + CurHeight*Areas[CurArea]->GetScale(), PlayerPos[2]};
	static int Framec = 0;
	Framec++;

	TimerVar++;
	ProcessingMask = false;

	// gravity, check for collisions, run conditions, etc, etc
	{
//		ForcedMoveCounter--;
//		printf("fmc: %d\n", ForcedMoveCounter);
	}
//	else
	{
		if(ForcedMoveCounter || DelayCounter)
		{
			if(ForcedMoveCounter)
			{
				ForcedMoveCounter--;
				MoveVec[1] = 0;
			}
		}

		if(!ForcedMoveCounter && !DelayCounter)
		{
			if(PlayMode == 1)
				MoveVec[1] -= 2.5f;

			if(Areas[CurArea]->FixCollisions(PlayerPos, MoveVec, GetRadius(), GetAscent(), GetDescent()))//PLAYER_WIDTH / Areas[CurArea]->GetScale(), (CurHeight*Areas[CurArea]->GetScale()) + PLAYER_ASCENT, MaxClimb * Areas[CurArea]->GetScale()))
			{
				if((LastHeight - PlayerPos[1] > MaxFall*Areas[CurArea]->GetScale()) && (PlayMode == 1))
					Variables[10] = (int)(LastHeight - PlayerPos[1] - MaxFall) / Areas[CurArea]->GetScale();
//				printf("ground! %d\n", Variables[10]);
				LastHeight = PlayerPos[1];
				MoveVec[1] = 0;
			}

//			if(!ProcessingMask)
			{
				if(Crouching)
					CurHeight += (CrouchHeight - CurHeight) * 0.2f;
				else
					CurHeight += (Start.Height - CurHeight) * 0.2f;

				float H = (Areas[CurArea]->GetHeadRoom(PlayerPos, PLAYER_WIDTH / Areas[CurArea]->GetScale())-PLAYER_ASCENT) / Areas[CurArea]->GetScale();
				if(H >= 0 && H < CurHeight)
					Crouching = true;
			}
		}
	}
	MoveVec[0] = MoveVec[2] = 0;

	// frames 0: update
	if(!(Framec&1))
	{
		if(ZXVars)
		{
			Variables[122] = (Variables[122]+1)&255;
			if(!Variables[122])
				Variables[123]++;
		}
		else
			Variables[19]++; //50 Hz timer
	}

	if(DelayCounter) DelayCounter--;

	if(!(Framec&3) && !DelayCounter)
	{
		int c = 3;
		while(c--)
		{
			PlayerAng[c] = fmod(PlayerAng[c], 360); if(PlayerAng[c] < 0) PlayerAng[c] += 360;
		}

		/* update some variables */
		if(ZXVars)
		{
			Variables[112] = ((int)PlayerPos[0])&255;
			Variables[113] = ((int)PlayerPos[0] >> 8)&255;
			Variables[114] = ((int)PlayerPos[1])&255;
			Variables[115] = ((int)PlayerPos[1] >> 8)&255;
			Variables[116] = ((int)PlayerPos[2])&255;
			Variables[117] = ((int)PlayerPos[2] >> 8)&255;
			Variables[118] = ((int)(PlayerAng[0] / 5))%72;
			Variables[119] = ((int)(PlayerAng[1] / 5))%72;
			Variables[120] = ((int)(PlayerAng[2] / 5))%72;
		}
		else
		{
			Variables[0] = (int)(PlayerPos[0] + 0.5f);
			Variables[1] = (int)(PlayerPos[1] + 0.5f);
			Variables[2] = (int)(PlayerPos[2] + 0.5f);
			Variables[3] = (int)(PlayerAng[0] + 0.5f);
			Variables[4] = (int)(PlayerAng[1] + 0.5f);
			Variables[5] = (int)(PlayerAng[2] + 0.5f);
		}
		PosDirty = false;

		// object & area conditions
		Areas[CurArea]->UpdateLogic(PPos);

		// global conditions
		GlobalArea->UpdateLogic(PPos);

		// clear mouse click variable
		if(!ZXVars) Variables[16] = 0;

		// check for player movement
		if(PosDirty)
		{
			float NewPlayerPos[3], NewPlayerAng[3];
			if(ZXVars)
			{
				PlayerPos[0] = (float)(Variables[112] | (Variables[113] << 8));
				PlayerPos[1] = (float)(Variables[114] | (Variables[115] << 8));
				PlayerPos[2] = (float)(Variables[116] | (Variables[117] << 8));
				PlayerAng[0] = (float)(Variables[118] * 5);
				PlayerAng[1] = (float)(Variables[119] * 5);
				PlayerAng[2] = (float)(Variables[120] * 5);
			}
			else
			{
				NewPlayerPos[0] = (float)Variables[0];
				LastHeight = NewPlayerPos[1] = (float)Variables[1];
				NewPlayerPos[2] = (float)Variables[2];
				NewPlayerAng[0] = (float)Variables[3];
				NewPlayerAng[1] = (float)Variables[4];
				NewPlayerAng[2] = (float)Variables[5];
			}

			if(
				fabs(NewPlayerPos[0] - PlayerPos[0]) < MAX_MOVE &&
				fabs(NewPlayerPos[1] - PlayerPos[1]) < MAX_MOVE &&
				fabs(NewPlayerPos[2] - PlayerPos[2]) < MAX_MOVE
				)
			{
				PlayerPosAdd[0] = (NewPlayerPos[0] - PlayerPos[0]) * 0.25f;
				PlayerPosAdd[1] = (NewPlayerPos[1] - PlayerPos[1]) * 0.25f;
				PlayerPosAdd[2] = (NewPlayerPos[2] - PlayerPos[2]) * 0.25f;

				// need to think about wrapping here
				int c = 3;
				while(c--)
				{
					NewPlayerAng[c] = fmod(NewPlayerAng[c], 360); if(NewPlayerAng[c] < 0) NewPlayerAng[c] += 360;

					NewPlayerAng[c] -= PlayerAng[c];

					float alternate;

					alternate = (NewPlayerAng[c] > 0) ? -(360 - NewPlayerAng[c]) : 360 + NewPlayerAng[c];
					if(fabs(NewPlayerAng[c]) < fabs(alternate)) PlayerAngAdd[c] = NewPlayerAng[c] * 0.25f; else PlayerAngAdd[c] = alternate * 0.25f;

					if(fabs(PlayerAngAdd[c]) <= 0.5f * 0.25f) PlayerAngAdd[c] = 0;
				}

				AddSteps = 4;
			}
			else
			{
				memcpy(PlayerPos, NewPlayerPos, sizeof(float)*3);
				memcpy(PlayerAng, NewPlayerAng, sizeof(float)*3);
			}
		}
	}

	Areas[CurArea]->Update();
	GlobalArea->Update();
	if(AddSteps)
	{
		AddSteps--;
		PlayerPos[0] += PlayerPosAdd[0];
		PlayerPos[1] += PlayerPosAdd[1];
		PlayerPos[2] += PlayerPosAdd[2];
		PlayerAng[0] += PlayerAngAdd[0];
		PlayerAng[1] += PlayerAngAdd[1];
		PlayerAng[2] += PlayerAngAdd[2];
	}
}

void CFreescapeGame::Delay(int c)
{
	DelayCounter = c * 2;
}

bool CFreescapeGame::StopProcessing() { return ProcessingMask;}

void CFreescapeGame::Reset()
{
	ProcessingMask = true;

	int c = MAX_AREAS;
	while(c--)
		if(Areas[c]) Areas[c]->Reset();
	GlobalArea->Reset();

	c = NUM_VARIABLES;
	while(c--)
		Variables[c] = 0;

	if(ZXVars)
		Variables[125] = 255;

	c = NUM_BITS;
	while(c--)
		Bits[c] = false;

	if(Start.ResetCondition)
	{
		CCondition *C = GlobalArea->GetCondition(Start.ResetCondition);
		if(C)
		{
			C->SetLooping(false);
			C->Execute(NULL);
		}
	}

	TimerVar = 0;
	ShotLastFrame = 0;
	
	CurHeight = Start.Height;
	VelocityY = 0;
	Crouching = false;
	DelayCounter = ForcedMoveCounter = 0;
	MessagePointer = 0;
	PlayMode = 1;

	AddSteps = 0;

	Goto(Start.Entrance, Start.Area);
}

void CFreescapeGame::ToggleCrouch()
{
	Crouching ^= true;
}

/*bool CFreescapeGame::FixCollisions(double *Pos, double *Move, double Radius, double Ascent, double Descent)
{
	if(Pos[1] + Move[1] <= 0)
	{
		Move[1] = -Pos[1];
		return true;
	}
	return false;
}*/

void CFreescapeGame::Draw()
{
	NumShooters = 0;
	GLdouble Projection[16];
	GLdouble ModelView[16];
	glGetDoublev(GL_PROJECTION_MATRIX, Projection);
	glPushMatrix();
		if(Areas[CurArea])
		{
			float PretendPos[3] = {PlayerPos[0], PlayerPos[1] + CurHeight*Areas[CurArea]->GetScale(), PlayerPos[2]};
			glRotatef(PlayerAng[2], 0, 0, 1);
			glRotatef(PlayerAng[0], 1, 0, 0);
			glRotatef(180+PlayerAng[1], 0, 1, 0);
			Areas[CurArea]->LoadScale();
			glTranslatef(-PlayerPos[0], -(PlayerPos[1] + CurHeight*Areas[CurArea]->GetScale()), -PlayerPos[2]);
			glGetDoublev(GL_MODELVIEW_MATRIX, ModelView);

			Areas[CurArea]->Draw(PretendPos);
		}
	glPopMatrix();

	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
		glVertex3f(-0.0025, 0, -0.1);
		glVertex3f(-0.00125, 0, -0.1);
		glVertex3f(0.0025, 0, -0.1);
		glVertex3f(0.00125, 0, -0.1);
		glVertex3f(0, -0.0025, -0.1);
		glVertex3f(0, -0.00125, -0.1);
		glVertex3f(0, 0.0025, -0.1);
		glVertex3f(0, 0.00125, -0.1);
	glEnd();

	if(ShotLastFrame)
	{
		ShotLastFrame--;
		
		if(ShotLastFrame >= SHOT_LASTDRAW)
		{
			glColor3f(flrand(), flrand(), flrand());
			glBegin(GL_LINES);
				glVertex3f(0, 0, -0.1);
				glVertex3f(40, -30, -0.1);
				glVertex3f(0, 0, -0.1);
				glVertex3f(-40, -30, -0.1);
			glEnd();
		}
	}

	float R = EBGF_GetHFOV() / EBGF_GetVFOV();
	if(MessagePointer)
	{
		int ms = 0;
		int Shuffle = 0;
		glPushMatrix();
			glTranslatef(-R*0.04f, -0.04, -0.1);
			glScalef(0.005, 0.005, 0.005);

			glTranslatef(0, MessagePointer, 0);
			while(ms < MessagePointer)
			{
				float I = (float)(MESSAGE_LIFE - Messages[ms].Age) / 25.0f;
				glColor4f(1, 1, 1, I);
				PrintFont->Print(Messages[ms].Text);
				if(Messages[ms].Age >= 0) Messages[ms].Age++;
				if(Messages[ms].Age > MESSAGE_LIFE)
					Shuffle++;
				ms++;
				glTranslatef(0, -1, 0);
			}
		glPopMatrix();

		while(Shuffle--)
		{
			ms = 0;
			while(ms < MessagePointer)
			{
				Messages[ms] = Messages[ms+1];
				ms++;
			}
			MessagePointer--;
		}
	}

	if(Handler)
	{
		glPushMatrix();
			glTranslatef(-R*0.04f, -0.04, -0.1);
			glScalef(0.005, 0.005, 0.005);
			Handler->DrawHud(PrintFont, Variables, R*16, 16);

		glPopMatrix();
			while(NumShooters--)
			{
				GLdouble OutX, OutY, OutZ;
				GLint ViewPort[] = {0, 0, 1, 1};
				gluProject(Shooters[NumShooters][0], Shooters[NumShooters][1], Shooters[NumShooters][2], ModelView, Projection, ViewPort, &OutX, &OutY, &OutZ);

				if(OutZ < 1 && OutX >= 0 && OutY >= 0 && OutX < 1 && OutY < 1)
				{
					glColor3f(flrand(), flrand(), flrand());
					glBegin(GL_LINES);
						glVertex3f(0.0825*R*(OutX - 0.5), 0.0825*(OutY - 0.5), -0.1);
						glVertex3f(0.0825*R*(0 - 0.5), 0.0825*(0 - 0.5), -0.1);

						glVertex3f(0.0825*R*(OutX - 0.5), 0.0825*(OutY - 0.5), -0.1);
						glVertex3f(0.0825*R*(0 - 0.5), 0.0825*(0 + 0.5), -0.1);

						glVertex3f(0.0825*R*(OutX - 0.5), 0.0825*(OutY - 0.5), -0.1);
						glVertex3f(0.0825*R*(0 + 0.5), 0.0825*(0 + 0.5), -0.1);

						glVertex3f(0.0825*R*(OutX - 0.5), 0.0825*(OutY - 0.5), -0.1);
						glVertex3f(0.0825*R*(0 + 0.5), 0.0825*(0 - 0.5), -0.1);
					glEnd();
				}
			}
	}
}

void CFreescapeGame::PrintMessage(char *Text, int Instrument)
{
	if(strlen(Text) > 255)
	{
		printf("ERROR: message too long [%s]\n", Text);
		return;
	}

	char OutputMessage[256];

	/* filter message - remove leading spaces and all \ns */
	bool NS = false;
	char *Ptr = OutputMessage;
	while(*Text)
	{
		switch(*Text)
		{
			case 27:	// 3d Construction Kit uses character 27 for backslash
				Text++;
			break;
			case ' ':
				if(NS) *Ptr++ = *Text;
			break;
			default:
				NS = true;
				*Ptr++ = *Text;
			break;
		}
		Text++;
	}
	*Ptr = '\0';

	/* if this leaves nothing to display then it was meant to clear the display — flag a message to disappear */
	if(!strlen(OutputMessage))
	{
		int c = 0;
		while(c < MessagePointer)
		{
			if(Messages[c].Age < MESSAGE_LIFE-MESSAGE_SHORTLIFE) break;
			c++;
		}
		if(c < MessagePointer)
			Messages[c].Age = MESSAGE_LIFE-MESSAGE_SHORTLIFE;
		return;
	}

	if(MessagePointer)
		if(!strcmp(Messages[MessagePointer-1].Text, OutputMessage))
		{
			Messages[MessagePointer-1].Age = 0;
			return;
		}

	if(MessagePointer == MAX_VISIBLE_MESSAGES)
	{
		int ms = 0;
		while(ms < MessagePointer)
		{
			Messages[ms] = Messages[ms+1];
			ms++;
		}
		MessagePointer--;
	}
	Messages[MessagePointer].Age = 0;

	strcpy(Messages[MessagePointer].Text, OutputMessage);
	MessagePointer++;
}

void CFreescapeGame::PostASCII(char key)
{
	if(ZXVars)
		Variables[121] = key;
	else
		Variables[15] = key;
}

void CFreescapeGame::SetMode(int m)
{
	PlayMode = m;
}

void CFreescapeGame::Assemble()
{
	int c = MAX_AREAS;
	while(c--)
	{
		CurArea = c;
		if(Areas[c])
			Areas[c]->Assemble();
	}
}

void CFreescapeGame::SetVariableMode(bool ZX)
{
	VarMask = ZX ? 255 : -1;
	ZXVars = ZX;
}

Sint32 CFreescapeGame::GetVariableMask()
{
	return VarMask;
}

void CFreescapeGame::SetHandler(CGameHandler *H)
{
	Handler = H;
}

void CFreescapeGame::PlaySound(int id)
{
	if(id >= 32) return;
	if(!Sounds[id])
	{
		printf("attempt to play sound %d\n", id);
		return;
	}
	Mix_PlayChannel(-1, Sounds[id], 0);
}

void CFreescapeGame::AddShot(float *Pos)
{
	if(NumShooters == MAX_SHOOTERS_PER_FRAME) return;
	Shooters[NumShooters][0] = Pos[0];
	Shooters[NumShooters][1] = Pos[1];
	Shooters[NumShooters][2] = Pos[2];
	NumShooters++;
}
