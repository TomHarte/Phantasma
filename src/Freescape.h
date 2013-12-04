#ifndef __FREESCAPE_H
#define __FREESCAPE_H

#include "SDL.h"
#include "SDL_opengl.h"
#include "ebgf/ebgf_Vector.h"
#include "ebgf/ebgf_Matrix.h"
#include "ebgf/ebgf_Font.h"
#include "ebgf/ebgf_DrawElements.h"
#include "ebgf/ebgf_InterleavedArrays.h"
#include "ebgf/ebgf_DataCollector.h"
#include "SDL_mixer.h"

class CFreescapeGame;
struct CFreescapeObject;

#define MAX_AREAS				256
#define MAX_OBJECTS_PER_AREA	512
#define MAX_CONDITIONS_PER_AREA	256
#define MAX_ANIMATORS_PER_AREA	256
#define MAX_ENTRANCES_PER_AREA	256

#define MAX_SHOOTERS_PER_FRAME	32

#define FCL_STACK_DEPTH			256

#define	MAX_FCL_LENGTH			65536

#define SENSORFLAG_UP			0x08
#define SENSORFLAG_DOWN			0x04
#define SENSORFLAG_NORTH		0x20
#define SENSORFLAG_SOUTH		0x10
#define SENSORFLAG_EAST			0x02
#define SENSORFLAG_WEST			0x01

#define MAX_VISIBLE_MESSAGES	9

#define OFFSET_DARKSIDE			0xc9ce
#define OFFSET_DRILLER			0xcf3e
#define OFFSET_TOTALECLIPSE		0xcdb7

#define NUM_VARIABLES			259
#define VAR_SHIELD				256
#define VAR_ENERGY				257
#define VAR_SCORE				258

#define NUM_BITS				256
#define NUM_SOUNDS				32

class CGameHandler
{
	public:
		virtual ~CGameHandler() {};
		virtual void DrawHud(CFont *Font, Sint32 *Variables, float Width, float Height) = 0;
};

class CFreescapeGame
{
	public:
		CFreescapeGame();
		~CFreescapeGame();

		bool SetPalette(char *name);
		void Set16PaletteGradient(float *Col1, float *Col2);
		bool OpenTXT(char *name);
		bool OpenZXBinary(char *name, int Offset);
		bool SetFont(char *name);
		bool LoadSounds(char *name);

		void Reset();
		void Draw();
		void Update();

		void Move(float x, float y, float z, float xr, float yr, float zr);
		void Activate();
		void Shoot();
		void ToggleCrouch();

		void SetupDisplay();
		void PostASCII(char key);

		void SetHandler(CGameHandler *H);

	private:
		CGameHandler *Handler;
		struct CColour
		{
			unsigned char Entry;
			GLfloat Col[3];
		};
		class CObject;
		class CCondition;
		class CArea;
		
		/* to configure variable mode */
		void SetVariableMode(bool ZXMode);

		/* functions for other classes to use */
		Sint32 GetVariable(unsigned int id);
		void SetVariable(unsigned int id, Sint32 value);
		bool GetBit(unsigned int id);
		void SetBit(unsigned int id, bool value);
		int GetCurrentArea();
		bool GetVis(unsigned int Object, unsigned int Area);
		void SetVis(unsigned int Object, unsigned int Area, bool Visible);
		void Destroy(unsigned int Object, unsigned int Area);
		bool QueryTimerTicked();
		CObject *GetObject(unsigned int id);
		void Goto(unsigned int Entrance, unsigned int Area);
		void SetAnimatorActive(unsigned int Animator, unsigned int Area, bool Active);
		void TriggerAnimator(unsigned int Animator);
		void Delay(int c);
		void PrintMessage(char *Text, int Instrument);
		void SetMode(int m);
		Sint32 GetVariableMask();
		void PlaySound(int id);
		void AddShot(float *Pos);
		void GetFrontVector(float *V);
		void GetSideVector(float *V);

		CMatrix ViewMatrix;

		CArea *GetCurArea();

		void Assemble();

		bool StopProcessing();
		bool ProcessingMask;

		/* other classes */
		enum ObjectStates {VISIBLE, INVISIBLE, DESTROYED};
		enum ObjectTypes {GROUP, SENSOR, CUBOID, PYRAMID, RECTANGLE, PLANAR};
		class CObject
		{
			public:
				CObject(CFreescapeGame *p);
				~CObject();

				void Move(float x, float y, float z);
				void MoveTo(float x, float y, float z);

				bool GetVis();
				void Destroy();
				void SetVis(bool v);

				void Draw(float *PlayerPos);
				void Update();
				void UpdateLogic(float *PlayerPos, float Scale);
				void Reset();

				bool GetCollided();		void SetCollided(bool);
				bool GetShot();			void SetShot(bool);
				bool GetActivated();	void SetActivated(bool);
				CCondition *GetCondition();
				void SetCondition(CCondition *C);

				float *GetPos();
				float *GetSize();

				void SetType(ObjectTypes Type);
				ObjectTypes GetType();
				void SetDefaultVisible(bool);
				void SetMoveable(bool);
				bool GetMoveable();
				void SetLocation(float *Pos, float *Size);
				void SetStartPos(float *Pos);
				void SetNumSides(unsigned int c);
				int GetNumSides();
				void SetVertex(unsigned int id, float *Pos);
				void SetPyramidType(unsigned int c);
				void SetApex(float *A, float *B);
				void SetColour(unsigned int, CColour);
				void SetSensorStats(int Speed, int Range, int DirFlags, int SensorID);
				void AddObject(unsigned int id);
				void SetArea(unsigned int id);

				void Assemble();
				bool CheckInside(float *Pos);

				float TestRayCollision(float *Start, float *Direction);
				float GetCollisionTime(float *Start, float *Move, float Radius, float Ascent, float Descent);
				bool FixLastCollision(float *Move);
				bool GetElevation(float *PPos, float Radius, float &El);

			private:
				/* the game to which this object belongs */
				CFreescapeGame *Parent;

				/* flags that are used during scripting */
				bool Collided, Shot, Activated;

				/* position and size stats */
				bool Moveable;
				float Pos[3], StartPos[3], Size[3];

				/* a few things related to whether the object is visible, invisble, destroyed, etc */
				ObjectStates CurState, DefaultState;

				/* two bits for tweening */
				float PosAdd[3];
				int AddSteps;

				/* two things related to collision detection */
				float HitNormal[3]; bool Supporting;

				/* the object type */
				ObjectTypes Type;

				/* some things used temporarily, during construction - need to break them off into a temp struct really */
				CColour Colours[6];
				float Verts[6][3];
				int NumSides;
				int PyramidType;
				float ApexStart[2], ApexEnd[2];

				/* a quick fix method of storing groups ... */
				unsigned int GroupSize, Area;
				int ObjectPtrs[MAX_OBJECTS_PER_AREA];

				/* a bunch of stats, used if this is a sensor object */
				int Speed, Range, DirFlags, SensorID;

				/* collision structs */
				struct Plane
				{
					CVector Normal;
					float Distance;
				};
				struct CollFace
				{
					public:
						bool SetNormal(CVector &N, float D);
						void AddEdge(CVector &V1, CVector &V2);
					Plane SitsOn;
					Plane Surrounders[6];
					int NumSurrounders;
				};

				/* local functions for building the graphic and collision shape */
				void BeginFace(CColour col);
				void AddVert(float *V);
				void AddVert(float x, float y, float z);
				void EndFace();

				/* the collision form */
				CollFace Faces[6];
				int NumFaces;

				/* the graphic */
				CInterleavedArrays *VArray;
				CDrawElements *Elements;
				float PolyOffset;

				/* stuff used for constructing the graphic */
				static CVector TempVArray[6];
				static int NumVerts;
				static CColour CurCol;
				static DataCollector<GLfloat> *VertexCollector;
				static DataCollector<GLuint> *IndexCollector;

				/* pointer to this object's script, if it has one at all */
				CCondition *Condition;

				/* function for checking if the player hits this object, and if so when */
				bool CheckIntercept(float *Start, float *Move, float I, float Radius, float Descent, float Ascent);

				/* counter for graphic display of shooting the player */
				int PlayerShot;
		};
		class CCondition
		{
			public:
				CCondition();
				~CCondition();

				void Clear();
				void AddInstruction(const char *Text);
				void Compile(bool Animator, CFreescapeGame *Game);

				void Reset();
				void SetActive(bool);
				void Trigger();
				void SetLooping(bool);

				bool Execute(CObject *Obj);

			private:
				/*
					STUFF TO DO WITH TOKENISING / COMPILING
				*/
				char *Program, *PPtr, *PreTok;
				int ErrNum;
				enum TokenTypes {
					ACTIVATEDQ = 0, ADDVAR, AGAIN, AND, ANDV, 
					COLLIDEDQ, 
					DELAY, DESTROY, DESTROYEDQ, 
					ELSE, END, ENDGAME, ENDIF, EXECUTE, 
					GOTO, 
					IF, INVIS, INVISQ, INCLUDE,
					LOOP,
					MODE, MOVE, MOVETO,
					NOTV,
					OR, ORV,
					GETXPOS, GETYPOS, GETZPOS, 
					PRINT,
					RESTART, REDRAW, REMOVE,
					SOUND, SETVAR, SHOTQ, START, STARTANIM, STOPANIM, SUBVAR, SYNCSND,
					THEN, TIMERQ, TOGVIS, TRIGANIM,
					UPDATEI,
					VAREQ, VARGQ, VARLQ, VISQ, VIS,
					WAIT, WAITTRIG,
					COMMA, OPENBRACKET, CLOSEBRACKET, CONSTANT, VARIABLE, STRINGLITERAL,
					UNKNOWN, ENDOFFILE,
					SETBIT, CLEARBIT, TOGGLEBIT, SWAPJET, BITNOTEQ, VARNOTEQ
				};
				struct Token
				{
					TokenTypes Type;

					union
					{
						int Value;
						char *String;
					} Data;

					Sint32 GetValue(CFreescapeGame *g, Sint32 Suggested = 0);
				};
				class FCLInstruction
				{
					public:
						FCLInstruction();
						~FCLInstruction();

						TokenTypes Type;
						union
						{
							struct
							{
								Token Source, Dest, Other;
							} TernaryOp;
							struct
							{
								Token Source, Dest;
							} BinaryOp;
							char *String;
							Token UnaryOp;
							struct
							{
								FCLInstruction *Passed, *Failed;
							} Then;
						} Data;

						FCLInstruction *Next;
				};
				bool RepeatToken;

				Token GetToken();
				void UnGetToken();
				void Expect(TokenTypes);
				void GetUnary(FCLInstruction *I);
				void GetBinary(FCLInstruction *I);
				void GetOptionalBinary(FCLInstruction *I);
				void GetTernary(FCLInstruction *I);
				void GetOptionalTernary(FCLInstruction *I);
				FCLInstruction *GetChain(bool EndIf = false);

				bool Animator;
				CFreescapeGame *Parent;

				/*
					the compiled program itself, and its state - plus functions that operate on these
				*/
				FCLInstruction *Head;
				int StackPtr;
				FCLInstruction *ProgramStack[FCL_STACK_DEPTH];
				bool Status;
				FCLInstruction *StartPos, *LoopPos;
				int StartStackPtr, LoopStackPtr, LoopCount;
				bool ObjFlags[MAX_OBJECTS_PER_AREA];
				bool Triggered, Looping;
				int Active;

				void GetTernaryValues(FCLInstruction *I, Sint32 &V1, Sint32 &V2, Sint32 &V3);
				void GetBinaryValues(FCLInstruction *I, Sint32 &V1, Sint32 &V2);
				void GetUnaryValue(FCLInstruction *I, Sint32 &V);
				bool QueryCondition(CObject *obj, FCLInstruction *Conditional);
				void ResetProg();
		};
		class CArea
		{
			public:
				CArea(CFreescapeGame *);
				~CArea();

				CObject *GetObject(unsigned int id);
				CCondition *GetAnimator(unsigned int id);
				CCondition *GetCondition(unsigned int id);

				void AddCondition(unsigned int id, CCondition *);
				void AddAnimator(unsigned int id, CCondition *);
				void AddObject(unsigned int id, CObject *);

				int FindFrontObject(float *Pos, float *Vec);
				float GetFrontDistance();

				void SetupDisplay();
				void Enter();
				void Draw(float *PlayerPos);
				void UpdateLogic(float *PlayerPos);
				void Update();
				void Reset();
				void Assemble();

				float GetScale();	// not sure about this...
				void SetScale(float s);
				void LoadScale();

				struct Entrance
				{
					float Pos[3], Rotation[3];
				};

				Entrance *GetEntrance(unsigned int id);
				void SetEntrance(unsigned int id, Entrance *);

				bool FixCollisions(float *Pos, float *Move, float Radius, float Ascent, float Descent);
				float GetHeadRoom(float *Pos, float Radius);

				void SetColours(CColour &Floor, CColour &Sky);
				void PushOut(float *Pos, float Radius, float Ascent, float Descent);

			private:
				CObject *Objects[MAX_OBJECTS_PER_AREA];
				CCondition *Conditions[MAX_CONDITIONS_PER_AREA];
				CCondition *Animators[MAX_ANIMATORS_PER_AREA];
				Entrance *Entrances[MAX_ENTRANCES_PER_AREA];

				float Scale;
				CColour Sky, Floor;
				bool HasFloor;
				CFreescapeGame *Parent;
				float DistanceToHit;
		};

		friend class Condition;

		/* things that actually make up this game */
		CArea *Areas[MAX_AREAS];
		CArea *GlobalArea;					// for global conditions
		Sint32 Variables[NUM_VARIABLES];	// all state variables
		Sint32 VarMask;
		bool ZXVars;
		bool PosDirty;
		float PlayerPos[3], PlayerAng[3];	// player position, and adders for tweening
		float PlayerPosAdd[3], PlayerAngAdd[3];
		int AddSteps;
		bool Bits[NUM_BITS];				// big bit field, for 8bit games
		int CurArea;						// the area the player is currently standing in
		struct
		{
			int Area, Entrance;
			int Step, Angle;
			int Vehicle, Height;
			int ResetCondition;
		} Start;							// the player's state at game start
		float Timer;						// number of ms between ticks
		unsigned int TimerVar;				// a counter that goes up with every logic update
		int MaxActivateDistance, MaxFall, MaxClimb;	// some maximum abilities
		int NumAreas;						// the number of areas in this game
		int ShotLastFrame;					// true if the next frame needs to have a laser effect...

		/* stuff for loading generally */
		float Palette[256][3];
		void MapColour(CColour *M);

		/* stuff for loading data from fsprint style TXT */
		char GetC();
		void UnGetC();
		const char *GetLine();
		int GetInt();
		float GetFloat();
		bool Expect (char *string);
		void SkipWhiteSpace(bool SkipNewLines = true);
		void SkipLine();
		CColour GetColour();
		bool Find(char *string);

		FILE *input;
		char charin[2], lastn;
		int charptr;
		int GIBase;
		
		/* ZX loading */
		CCondition *GetConditionZXBinary(Uint8 *Ptr, int Len, bool print = false);

		/* some functions for game update */
		float VelocityY, CurHeight, CrouchHeight, LastHeight;
		float FindFrontObject(int &Area, int &Object);
		bool Crouching;
		float MoveVec[3];
		int DelayCounter;
		int ForcedMoveCounter;
		int PlayMode;

		struct
		{
			int Age;
			char Text[256];
		} Messages[MAX_VISIBLE_MESSAGES];
		int MessagePointer;

		Mix_Chunk *Sounds[NUM_SOUNDS];
		GLfloat Shooters[MAX_SHOOTERS_PER_FRAME][3];
		int NumShooters;
		CFont *PrintFont;
};

#endif
