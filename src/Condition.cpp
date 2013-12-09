#include "Condition.h"


void CFreescapeGame::CCondition::UnGetToken()
{
	RepeatToken = true;
}

CFreescapeGame::CCondition::Token CFreescapeGame::CCondition::GetToken()
{
	static Token R;
	if(RepeatToken)
	{
		RepeatToken = false;
		return R;
	}

	// skip white space first
	while(*PPtr == ' ' || *PPtr == '\t' || *PPtr == '\n')
		PPtr++;
	PreTok = PPtr;

	if(!(*PPtr))
	{
		R.Type = ENDOFFILE;
		return R;
	}

	struct TokTable
	{
		char *Name;
		TokenTypes Type;
	} Tokens[] =
	{
		{"setbit", SETBIT},			{"clrbit", CLEARBIT},		{"togbit", TOGGLEBIT},	{"swapjet", SWAPJET},
		{"bit!=?", BITNOTEQ},		{"var!=?", VARNOTEQ},
		{"activated?", ACTIVATEDQ},	{"act?", ACTIVATEDQ},		{"addvar", ADDVAR},		{"add", ADDVAR},
		{"again", AGAIN},			{"and", AND},				{"andv", ANDV},			{"collided?", COLLIDEDQ},
		{"col?", COLLIDEDQ},		{"delay", DELAY},			{"destroy", DESTROY},	{"destroyed?", DESTROYEDQ},
		{"else", ELSE},				{"endgame", ENDGAME},		{"endif", ENDIF},		{"end", END},
		{"execute", EXECUTE},		{"ex", EXECUTE},			{"goto", GOTO},			{"if", IF},
		{"include", INCLUDE},		{"invis?", INVISQ},			{"invis", INVIS},		{"iv", INVIS},
		{"loop", LOOP},				{"mode", MODE},				{"moveto", MOVETO},		{"move", MOVE},
		{"notv", NOTV},				{"orv", ORV},				{"or", OR},				{"getxpos", GETXPOS},
		{"getypos", GETYPOS},		{"getzpos", GETZPOS},		{"print", PRINT},		{"restart", RESTART},
		{"redraw", REDRAW},			{"remove", REMOVE},			{"sound", SOUND},		{"setvar", SETVAR},
		{"shot?", SHOTQ},			{"startanim", STARTANIM},	{"start", START},		{"stopanim", STOPANIM},
		{"subvar", SUBVAR},			{"sub", SUBVAR},			{"syncsnd", SYNCSND},	{"then", THEN},
		{"timer?", TIMERQ},			{"togvis", TOGVIS},			{"tog", TOGVIS},		{"triganim", TRIGANIM},
		{"updatei", UPDATEI},		{"var=?", VAREQ},			{"var>?", VARGQ},		{"var<?", VARLQ},
		{"v=?", VAREQ},				{"v>?", VARGQ},				{"v<?", VARLQ},			{"vis?", VISQ},
		{"vis", VIS},				{"waittrig", WAITTRIG},		{"wait", WAIT},			{",", COMMA},
		{"(", OPENBRACKET},			{")", CLOSEBRACKET},
		{NULL}
	};
	TokTable *P = Tokens;
	while(P->Name)
	{
		if(!strncmp(P->Name, PPtr, strlen(P->Name)))
		{
			R.Type = P->Type;
			PPtr += strlen(P->Name);
			return R;
		}
		P++;
	}

	// maybe a string literal?
	if(*PPtr == '"')
	{
		PPtr++;
		R.Type = STRINGLITERAL;
		R.Data.String = PPtr;
		while(*PPtr != '"') PPtr++;
		*PPtr = '\0'; PPtr++;
		R.Data.String = strdup(R.Data.String);
		return R;
	}

	//or a variable?
	bool Var = false;
	int Multiplier = 1;
	R.Type = CONSTANT;
	if(*PPtr == 'v')
	{
		R.Type = VARIABLE;
		PPtr++;
	}
	if(*PPtr == '-')
	{
		Multiplier = -1;
		PPtr++;
	}
	R.Data.Value = 0;
	while(*PPtr >= '0' && *PPtr <= '9')
	{
		R.Data.Value = (R.Data.Value*10) + (*PPtr - '0');
		PPtr++;
		Var = true;
	}
	R.Data.Value *= Multiplier;

	if(!Var) R.Type = UNKNOWN;
	return R;
}

void CFreescapeGame::CCondition::Expect(TokenTypes tok)
{
	Token NewT = GetToken();
	if(NewT.Type != tok)
		ErrNum = 1;
}

CFreescapeGame::CCondition::FCLInstruction::~FCLInstruction()
{
	if(Next)
		delete Next;
	if(Type == THEN)
	{
		delete Data.Then.Passed;
		delete Data.Then.Failed;
	}
}

CFreescapeGame::CCondition::FCLInstruction::FCLInstruction()
{
	Next = NULL;
	Type = UNKNOWN;
	Data.Then.Passed = Data.Then.Failed = NULL;
}

void CFreescapeGame::CCondition::GetUnary(FCLInstruction *I)
{
	Expect(OPENBRACKET);
	I->Data.UnaryOp = GetToken();
	Expect(CLOSEBRACKET);
}

void CFreescapeGame::CCondition::GetBinary(FCLInstruction *I)
{
	Expect(OPENBRACKET);
	I->Data.BinaryOp.Source = GetToken();
	Expect(COMMA);
	I->Data.BinaryOp.Dest = GetToken();
	Expect(CLOSEBRACKET);
}

void CFreescapeGame::CCondition::GetOptionalBinary(FCLInstruction *I)
{
	Expect(OPENBRACKET);
	I->Data.BinaryOp.Source = GetToken();
	if(GetToken().Type == COMMA)
	{
		I->Data.BinaryOp.Dest = GetToken();
		Expect(CLOSEBRACKET);
	}
	else
		I->Data.BinaryOp.Dest.Type = UNKNOWN;
}

void CFreescapeGame::CCondition::GetTernary(FCLInstruction *I)
{
	Expect(OPENBRACKET);
	I->Data.TernaryOp.Source = GetToken();
	Expect(COMMA);
	I->Data.TernaryOp.Dest = GetToken();
	Expect(COMMA);
	I->Data.TernaryOp.Other = GetToken();
	Expect(CLOSEBRACKET);
}

void CFreescapeGame::CCondition::GetOptionalTernary(FCLInstruction *I)
{
	Expect(OPENBRACKET);
	I->Data.TernaryOp.Source = GetToken();
	Expect(COMMA);
	I->Data.TernaryOp.Dest = GetToken();
	if(GetToken().Type == COMMA)
	{
		I->Data.TernaryOp.Other = GetToken();
		Expect(CLOSEBRACKET);
	}
	else
		I->Data.TernaryOp.Other.Type = UNKNOWN;
}

CFreescapeGame::CCondition::FCLInstruction *CFreescapeGame::CCondition::GetChain(bool EndIf)
{
	FCLInstruction *NewChain, **CurPtr;
	CurPtr = &NewChain; NewChain = NULL;

	while(1)
	{
		Token NewTok = GetToken();
		if(NewTok.Type == ENDOFFILE) break;

		*CurPtr = new FCLInstruction;
		(*CurPtr)->Next = NULL;

		switch((*CurPtr)->Type = NewTok.Type)
		{
			/*
				ops with no argument at all
			*/
			case SWAPJET:
			case REDRAW:	case END:			case ENDGAME:	case ACTIVATEDQ:
			case COLLIDEDQ:	case TIMERQ:		case AND:		case OR:
			case WAIT:		case SHOTQ:			case IF:		case RESTART:
			case START:		case WAITTRIG:		case AGAIN:
			break;
			/*
				definite unary ops
			*/
			case SETBIT:	case CLEARBIT:		case TOGGLEBIT:
			case LOOP:		case UPDATEI:		case DELAY:		case EXECUTE:
			case INCLUDE:	case MODE:			case SOUND:		case SYNCSND:
			case TRIGANIM:	case REMOVE:
				GetUnary(*CurPtr);
			break;
			/*
				definite binary ops
			*/
			case BITNOTEQ:	case VARNOTEQ:
			case VAREQ:		case VARLQ:			case VARGQ:		case SETVAR:
			case SUBVAR:	case ADDVAR:		case PRINT:		case ANDV:
			case ORV:		case NOTV:
 				GetBinary(*CurPtr);
			break;
			/*
				things that may be binary ops but may be unary ops
			*/
			case INVIS:		case VIS:			case GOTO:		case DESTROY:
			case TOGVIS:	case DESTROYEDQ:	case VISQ:		case INVISQ:
			case STARTANIM:	case STOPANIM:
				GetOptionalBinary(*CurPtr);
			break;
			/*
				definite ternary ops (for animators only...)
			*/
			case MOVE:		case MOVETO:
				GetTernary(*CurPtr);
			break;
			/*
				things that may be ternary ops but may be binary ops
			*/
			case GETXPOS:	case GETYPOS:	case GETZPOS:
				GetOptionalTernary(*CurPtr);
			break;
			/*
				stuff for handling conditional parts of programs
			*/
			case THEN:
				(*CurPtr)->Data.Then.Passed = GetChain(true);
				Token LocT = GetToken();
				if(LocT.Type == ELSE)
					(*CurPtr)->Data.Then.Failed = GetChain(true);
				else
					UnGetToken();
			break;
			case ENDIF:
				if(EndIf)			/* COMPATIBILITY: endif is ignored if we're at the top level */
					return NewChain;
			break;
			case ELSE:
				if(EndIf)
				{
					UnGetToken();
					return NewChain;
				}
			default:
				printf("don't know how to handle token at [%10s]\n", PreTok);
				printf("[[%s]]\n", Program);
				return NewChain;
			break;
		}
		CurPtr = &(*CurPtr)->Next;
	}
	return NewChain;
}

void CFreescapeGame::CCondition::SetLooping(bool c)
{
	Looping = c;
	if(!c && Active < 0) Active = 1;
}

void CFreescapeGame::CCondition::AddInstruction(const char *Text)
{
	if(!Program)
	{
		PPtr = Program = new char[MAX_FCL_LENGTH];
	}
	strcpy(PPtr, Text);
	PPtr += strlen(PPtr); PPtr[0] = '\n'; PPtr[1] = '\0'; PPtr++;
}

void CFreescapeGame::CCondition::Compile(bool Anim, CFreescapeGame *P)
{
	Animator = Anim; Parent = P;

	PPtr = Program;
	if(!PPtr) return;
	ErrNum = 0;

	Head = GetChain();

	delete[] Program; Program = NULL;
}
/*CFreescapeGame::CCondition::FCLInstruction::~FCLInstruction()
{
	if(Next)
		delete Next;
	if(Type == THEN)
	{
		delete Data.Then.Passed;
		delete Data.Then.Failed;
	}
}*/
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

CFreescapeGame::CCondition::CCondition()
{
	Head = NULL;
	Program = NULL;
	RepeatToken = false;
	Looping = true;
	ResetProg();
}

CFreescapeGame::CCondition::~CCondition()
{
	Clear();
}

void CFreescapeGame::CCondition::Clear()
{
	delete Head; Head = NULL;
	delete[] Program; Program = NULL;
	RepeatToken = false;
}

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

Sint32 CFreescapeGame::CCondition::Token::GetValue(CFreescapeGame *g, Sint32 Suggested)
{
	if(Type == CONSTANT) return Data.Value;
	if(Type == VARIABLE) return g->GetVariable(Data.Value);
	return Suggested;
}

void CFreescapeGame::CCondition::GetTernaryValues(FCLInstruction *I, Sint32 &Var1, Sint32 &Var2, Sint32 &Var3)
{
	Var1 = I->Data.TernaryOp.Source.GetValue(Parent, Var1);
	Var2 = I->Data.TernaryOp.Dest.GetValue(Parent, Var2);
	Var3 = I->Data.TernaryOp.Other.GetValue(Parent, Var3);
}

void CFreescapeGame::CCondition::GetBinaryValues(FCLInstruction *I, Sint32 &Var1, Sint32 &Var2)
{
	Var1 = I->Data.BinaryOp.Source.GetValue(Parent, Var1);
	Var2 = I->Data.BinaryOp.Dest.GetValue(Parent, Var2);
}

void CFreescapeGame::CCondition::GetUnaryValue(FCLInstruction *I, Sint32 &Var)
{
	Var = I->Data.UnaryOp.GetValue(Parent, Var);
}

bool CFreescapeGame::CCondition::QueryCondition(CObject *obj, FCLInstruction *Conditional)
{
	Sint32 Var1, Var2;
	bool Res = false;
	switch(Conditional->Type)
	{
		default:
			printf("Unknown conditional %s\n", ConNames[(int)Conditional->Type]);
		break;
		case COLLIDEDQ:
			Res = obj ? obj->GetCollided() : true;
			if(obj) obj->SetCollided(false);
		return Res;
		case SHOTQ:
			Res = obj ? obj->GetShot() : false;
			if(obj) obj->SetShot(false);
		return Res;
		case ACTIVATEDQ:
			Res = obj ? obj->GetActivated() : false;
			obj->SetActivated(false);	/*not sure about this */
		return Res;
		case VISQ:
			Var2 = Parent->GetCurrentArea();
			GetBinaryValues(Conditional, Var1, Var2);
		return Parent->GetVis(Var1, Var2);
		case INVISQ:
			Var2 = Parent->GetCurrentArea();
			GetBinaryValues(Conditional, Var1, Var2);
		return !Parent->GetVis(Var1, Var2);
		case VAREQ:
			GetBinaryValues(Conditional, Var1, Var2);
		return (int)Var1 == (int)Var2;
		case VARNOTEQ:
			GetBinaryValues(Conditional, Var1, Var2);
		return (int)Var1 != (int)Var2;
		case BITNOTEQ:
			GetBinaryValues(Conditional, Var1, Var2);
			bool B = Parent->GetBit(Var1);
		return B == (Var2 ? true : false);
		case VARLQ:
			GetBinaryValues(Conditional, Var1, Var2);
		return Var1 < Var2;
		case VARGQ:
			GetBinaryValues(Conditional, Var1, Var2);
		return Var1 > Var2;
		case TIMERQ:
		return Parent->QueryTimerTicked();
		case ADDVAR:
			GetBinaryValues(Conditional, Var1, Var2);
			Var2 += Var1;
			if(Conditional->Data.BinaryOp.Dest.Type == VARIABLE)
				Parent->SetVariable(Conditional->Data.BinaryOp.Dest.Data.Value, Var2);
		return (Var2&Parent->GetVariableMask()) ? true : false;
		case SUBVAR:
			GetBinaryValues(Conditional, Var1, Var2);
			Var2 -= Var1;
			if(Conditional->Data.BinaryOp.Dest.Type == VARIABLE)
				Parent->SetVariable(Conditional->Data.BinaryOp.Dest.Data.Value, Var2);
		return (Var2&Parent->GetVariableMask()) ? true : false;
	}
	return Res;
}

bool CFreescapeGame::CCondition::Execute(CObject *obj)
{
	Sint32 Var1, Var2, Var3, Mask;

	Mask = Parent->GetVariableMask();
	if(!Head || !Active) {return true;}
//	if(Animator) printf("anim %d\n", Active);

	/* oh man! */
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
				/* execute someone else's script as though mine */
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

			/* addvar & subvar. Not sure if they really affect Status here */
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

			/*
				loops
			*/
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

			/*
				Animator specific bits
			*/
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
}
