//
//  SyntaxBuilder.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "SyntaxBuilder.h"

void CCondition::UnGetToken()
{
	RepeatToken = true;
}

CCondition::Token CFreescapeGame::CCondition::GetToken()
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

std::vector<FCLInstruction> *GetInstructions(std::string source)
{
	std::vector<FCLInstruction> *Instructions = new std::vector<FCLInstruction>;

	return Instructions;
}
