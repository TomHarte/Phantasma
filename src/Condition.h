//
//  Condition.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef Phantasma_Condition_h
#define Phantasma_Condition_h

#import "GameState.h"
#import <vector>

class FCLInstruction;

class CCondition
{
	public:
		CCondition(CGameState *gameState, std::vector<FCLInstruction *> *instructions);
		virtual ~CCondition();

/*		void Clear();
		void AddInstruction(const char *Text);
		void Compile(bool Animator, CFreescapeGame *Game);

		void Reset();
		void SetActive(bool);
		void Trigger();
		void SetLooping(bool);

		bool Execute(CObject *Obj);*/

	private:

		/*bool Animator;
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
