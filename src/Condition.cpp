#include "Condition.h"
#include "Instruction.h"

CCondition::CCondition(CGameState &_gameState, FCLInstructionVector _instructions, bool _isAnimator)
{
	gameState = &_gameState;
	instructions = _instructions;
	isAnimator = _isAnimator;
}

bool CCondition::statusOfConditional(FCLInstruction &conditional)	//CObject *obj,
{
	int32_t var1, var2;
	bool result = false;

	switch(conditional.getType())
	{
		default:
			std::cerr << "Unknown conditional" << &conditional;
		break;

		/*
			Collided, shot and activated all atomically test and reset object state bits;
			so they delegate determining the result to the object itself
		*/
//		case COLLIDEDQ:
//			Res = obj ? obj->GetCollided() : true;
//			if(obj) obj->SetCollided(false);
//		return Res;
//		case SHOTQ:
//			Res = obj ? obj->GetShot() : false;
//			if(obj) obj->SetShot(false);
//		return Res;
//		case ACTIVATEDQ:
//			Res = obj ? obj->GetActivated() : false;
//			obj->SetActivated(false);	// not sure about this
//		return Res;

		/*
			Visibility and invisibility need working objects ...
		*/
//		case VISQ:
//			Var2 = Parent->GetCurrentArea();
//			GetBinaryValues(Conditional, Var1, Var2);
//		return Parent->GetVis(Var1, Var2);
//		case INVISQ:
//			Var2 = Parent->GetCurrentArea();
//			GetBinaryValues(Conditional, Var1, Var2);
//		return !Parent->GetVis(Var1, Var2);

		/*
			Some easy ones — just compare one value to another
		*/
		case Token::VAREQ:
			conditional.getValue(*gameState, var1, var2);
		return var1 == var2;

		case Token::VARNOTEQ:
			conditional.getValue(*gameState, var1, var2);
		return var1 != var2;

		case Token::VARLQ:
			conditional.getValue(*gameState, var1, var2);
		return var1 < var2;

		case Token::VARGQ:
			conditional.getValue(*gameState, var1, var2);
		return var1 > var2;

		// bit not equal, which this code implements via the reserved word
		// bit!=?, is added to facilitate running of 8-bit games — syntax is
		// bit!=? (bit number, value), with the two things treated as Booleans
		case Token::BITNOTEQ:
			conditional.getValue(*gameState, var1, var2);
			bool B = gameState->getBit(var1);
		return B == (var2 ? true : false);



//		case TIMERQ:
//		return Parent->QueryTimerTicked();
//		case ADDVAR:
//			GetBinaryValues(Conditional, Var1, Var2);
//			Var2 += Var1;
//			if(Conditional->Data.BinaryOp.Dest.Type == VARIABLE)
//				Parent->SetVariable(Conditional->Data.BinaryOp.Dest.Data.Value, Var2);
//		return (Var2&Parent->GetVariableMask()) ? true : false;
//		case SUBVAR:
//			GetBinaryValues(Conditional, Var1, Var2);
//			Var2 -= Var1;
//			if(Conditional->Data.BinaryOp.Dest.Type == VARIABLE)
//				Parent->SetVariable(Conditional->Data.BinaryOp.Dest.Data.Value, Var2);
//		return (Var2&Parent->GetVariableMask()) ? true : false;
	}

	return result;
}


/*void CFreescapeGame::CCondition::SetLooping(bool c)
{
	Looping = c;
	if(!c && Active < 0) Active = 1;
}

const char *ConNames[] =
{
	"activated?", "addvar", "again", "and", "andv", "collided?", "delay", "destroy", "destroyed?", "else",
	"end", "endgame", "endif", "execute", "goto", "if", "invis", "invis?", "include", "loop", "mode",
	"move", "moveto", "notv", "or", "orv", "getxpos", "getypos", "getzpos", "print", "restart", "redraw",
	"remove", "sound", "setvar", "shot?", "start", "startanim", "stopanim", "subvar", "syncsnd", "then",
	"timer?", "togvis", "triganim", "updatei", "var=?", "var>?", "var<?", "vis?", "vis", "wait",
	"waittrig", ",", "(", ")", "constant", "variable", "literal", "???", "EOF", "setbit", "clrbit", "togbit",
	"swapjet", "bit!=?", "var!=?"
};

void CFreescapeGame::CCondition::ResetProg()
{
	if(Active > 0) Active--;
	if(!Active && Looping) Active = -1;
	ProgramStack[0] = Head;
	StackPtr = 0;
}

void CFreescapeGame::CCondition::Reset()
{
	Triggered = false;
	Active = (Animator || !Looping) ? 0 : -1;
	ResetProg();
	int c = MAX_OBJECTS_PER_AREA;
	while(c--)
		ObjFlags[c] = false;
}

void CFreescapeGame::CCondition::SetActive(bool s)	{ if(s && Active >= 0) Active++; if(!s) Active = 0;}
void CFreescapeGame::CCondition::Trigger()			{ Triggered = true; }

bool CFreescapeGame::CCondition::Execute(CObject *obj)
{
	Sint32 Var1, Var2, Var3, Mask;

	Mask = Parent->GetVariableMask();
	if(!Head || !Active) {return true;}
//	if(Animator) printf("anim %d\n", Active);

	// oh man!
	while(ProgramStack[StackPtr])
	{
		FCLInstruction *Instr = ProgramStack[StackPtr];
		ProgramStack[StackPtr] = ProgramStack[StackPtr]->Next;

		switch(Instr->Type)
		{
			default:
				printf("Unhandled FCLInstruction %s\n", ConNames[(int)Instr->Type]);
				ResetProg();
			return false;

			case IF:
			{
				FCLInstruction *Conditional = ProgramStack[StackPtr];
				ProgramStack[StackPtr] = ProgramStack[StackPtr]->Next;
				Status = QueryCondition(obj, Conditional);
			}
			break;
			case AND:
			{
				FCLInstruction *Conditional = ProgramStack[StackPtr];
				ProgramStack[StackPtr] = ProgramStack[StackPtr]->Next;
				Status = Status && QueryCondition(obj, Conditional);
			}
			break;
			case OR:
			{
				FCLInstruction *Conditional = ProgramStack[StackPtr];
				ProgramStack[StackPtr] = ProgramStack[StackPtr]->Next;
				Status = Status || QueryCondition(obj, Conditional);
			}
			break;

			case THEN:
				if(Status && Instr->Data.Then.Passed)
				{
					// execute passed
					ProgramStack[StackPtr+1] = Instr->Data.Then.Passed;
					StackPtr++;
				}
				
				if(!Status && Instr->Data.Then.Failed)
				{
					// execute failed
					ProgramStack[StackPtr+1] = Instr->Data.Then.Failed;
					StackPtr++;
				}
			break;

			case ELSE:
			case ENDIF:
				if(StackPtr > 0)
					StackPtr--;
				else
					if(ProgramStack[StackPtr])
					{
						printf("too many ENDIFs\n");
						ResetProg();
						return false;
					}
			break;

			case EXECUTE:
				// execute someone else's script as though mine
				GetUnaryValue(Instr, Var1);
//				printf("execute %d\n", (int)Var1);
				CObject *NewObj = Parent->GetObject(Var1);
				ResetProg();
				if(NewObj)
				{
					CCondition *C = NewObj->GetCondition();
					if(C)
					{
						C->SetActive(true);
						return C->Execute(obj);
					}
					return true;
				}
				return false;
			break;
			case END:
				if(!Animator) ResetProg();
			return true;
			case WAIT:
			return true;

			case VIS:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetVis(Var1, Var2, true);
			break;
			case INVIS:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetVis(Var1, Var2, false);
			break;
			case DESTROY:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->Destroy(Var1, Var2);
			break;
			case GOTO:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->Goto(Var1, Var2);
			break;
			case TOGVIS:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetVis(Var1, Var2, Parent->GetVis(Var1, Var2) ^ true);
			break;
			case STARTANIM:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetAnimatorActive(Var1, Var2, true);
//				printf("startanim %d, %d\n", (int)Var1, (int)Var2);
			break;
			case STOPANIM:
				Var2 = Parent->GetCurrentArea();
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetAnimatorActive(Var1, Var2, false);
//				printf("stopanim %d, %d\n", (int)Var1, (int)Var2);
			break;
			case TRIGANIM:
				GetUnaryValue(Instr, Var1);
				Parent->TriggerAnimator(Var1);
//				printf("triganim %d\n",  (int)Var1);
			break;

			case CLEARBIT:
				GetUnaryValue(Instr, Var1);
				Parent->SetBit(Var1, false);
			break;
			case SETBIT:
				GetUnaryValue(Instr, Var1);
				Parent->SetBit(Var1, true);
			break;
			case TOGGLEBIT:
				GetUnaryValue(Instr, Var1);
				Parent->SetBit(Var1, Parent->GetBit(Var1)^true);
			break;

			case SETVAR:
				GetBinaryValues(Instr, Var1, Var2);
				Parent->SetVariable(Instr->Data.BinaryOp.Dest.Data.Value, Var1);
			break;

			// addvar & subvar. Not sure if they really affect Status here
			case ADDVAR:
				GetBinaryValues(Instr, Var1, Var2);
				Var2 += Var1;
				if(Instr->Data.BinaryOp.Dest.Type == VARIABLE)
					Parent->SetVariable(Instr->Data.BinaryOp.Dest.Data.Value, Var2);
				Status = (Var2&Mask) ? true : false;
			break;
			case SUBVAR:
				GetBinaryValues(Instr, Var1, Var2);
				Var2 -= Var1;
				if(Instr->Data.BinaryOp.Dest.Type == VARIABLE)
					Parent->SetVariable(Instr->Data.BinaryOp.Dest.Data.Value, Var2);
				Status = (Var2&Mask) ? true : false;
			break;

			case WAITTRIG:
				if(Animator && !Triggered)
				{
					ProgramStack[StackPtr] = Instr;
					return true;
				}
				Triggered = false;
			break;

			//
			//	loops
			//
			case LOOP:
				GetUnaryValue(Instr, Var1);
				LoopPos = ProgramStack[StackPtr];
				LoopStackPtr = StackPtr;
				LoopCount = (int)Var1;
			break;
			case AGAIN:
				LoopCount--;
				if(LoopCount)	//seems to be the test, not > 0 as the documentation claims
				{
					StackPtr = LoopStackPtr;
					ProgramStack[StackPtr] = LoopPos;
				}
			return true;

			//
			//	Animator specific bits
			//
			case START:
				if(Animator)
				{
					StartPos = ProgramStack[StackPtr];
					StartStackPtr = StackPtr;
				}
			break;
			case RESTART:
				if(Animator)
				{
					StackPtr = StartStackPtr;
					ProgramStack[StackPtr] = StartPos;
					return true;	// I think?
				}
			break;
			case INCLUDE:
				if(Animator)
				{
					GetUnaryValue(Instr, Var1);
					ObjFlags[(int)Var1] = true;
				}
			break;
			case MOVETO:
				if(Animator)
				{
					GetTernaryValues(Instr, Var1, Var2, Var3);
					int c = 256;
					while(c--)
					{
						if(ObjFlags[c])
						{
							CObject *NewObj = Parent->GetObject(c);
							if(NewObj)
								NewObj->MoveTo(Var1, Var2, Var3);
						}
					}
				}
			return true;
			case MOVE:
				if(Animator)
				{
					GetTernaryValues(Instr, Var1, Var2, Var3);
					int c = 256;
					while(c--)
					{
						if(ObjFlags[c])
						{
							CObject *NewObj = Parent->GetObject(c);
							if(NewObj)
								NewObj->Move(Var1, Var2, Var3);
						}
					}
				}
			return true;

			case SOUND:
			case SYNCSND:
				GetUnaryValue(Instr, Var1);
				Parent->PlaySound(Var1-1); // should subtract 1?
//				printf("ignored sound\n");
			break;
			case UPDATEI:
//				printf("instrument update ignored\n");
			break;
			case REDRAW:	//always ignore redraw
			return true;
			case DELAY:		//always ignore delay - it's a bad solution
				GetUnaryValue(Instr, Var1);
				Parent->Delay(Var1);
			return true;
			case PRINT:		// ignore for now
				Parent->PrintMessage(Instr->Data.BinaryOp.Source.Data.String, 0);
//				printf("%s\n", Instr->Data.BinaryOp.Source.Data.String);
			break;
			case MODE:
				GetUnaryValue(Instr, Var1);
				Parent->SetMode(Var1);
			break;

			case ENDGAME:
				Parent->Reset();
			return true;
		}
	}

	// if we got to here then we reached the end of the program, so set things back to default for next time
	ResetProg();
	return true;
}*/
