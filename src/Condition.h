//
//  Condition.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef Phantasma_Condition_h
#define Phantasma_Condition_h

class CCondition
{
	public:
		CCondition();
		~CCondition();

/*		void Clear();
		void AddInstruction(const char *Text);
		void Compile(bool Animator, CFreescapeGame *Game);

		void Reset();
		void SetActive(bool);
		void Trigger();
		void SetLooping(bool);

		bool Execute(CObject *Obj);*/

	private:
		/*
			STUFF TO DO WITH TOKENISING / COMPILING
		*/
/*		char *Program, *PPtr, *PreTok;
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
		CFreescapeGame *Parent;*/

		/*
			the compiled program itself, and its state - plus functions that operate on these
		*/
/*		FCLInstruction *Head;
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
		void ResetProg();*/
};


#endif
