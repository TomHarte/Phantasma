//
//  SyntaxBuilder.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "InstructionBuilder.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>

class CSyntaxBuilder
{
	private:
		bool RepeatLastToken;
		Token *LastToken;
		uint32_t errorNumber;

		size_t CodePointer;
		std::string *SourceCode;
	
		// we implement a single token pushback; this is
		// achieved by just keeping the value of LastToken
		// that we already have
		void UnGetToken()
		{
			RepeatLastToken = true;
		}

		Token *GetToken()
		{
			if(RepeatLastToken)
			{
				RepeatLastToken = false;
				return LastToken;
			}

			// skip white space
			while((*SourceCode)[CodePointer] == ' ' || (*SourceCode)[CodePointer] == '\t' || (*SourceCode)[CodePointer] == '\n')
				CodePointer++;

			// note where we are now, so we can report a meaningful error later if required
			size_t CodePointerBeforeToken = CodePointer;

			// check whether we've hit the end of the string
			if((*SourceCode)[CodePointer] == '\0')
			{
				LastToken = new Token(Token::ENDOFFILE);
				return LastToken;
			}

			// this is a table mapping the FCL reserved words to entries in our Type enum
			struct TokenMapping
			{
				const char *name;
				enum Token::Type type;
			} Tokens[] =
			{
				{"setbit",		Token::SETBIT},			{"clrbit",		Token::CLEARBIT},		{"togbit",		Token::TOGGLEBIT},	{"swapjet",		Token::SWAPJET},
				{"bit!=?",		Token::BITNOTEQ},		{"var!=?",		Token::VARNOTEQ},
				{"activated?",	Token::ACTIVATEDQ},		{"act?",		Token::ACTIVATEDQ},		{"addvar",		Token::ADDVAR},		{"add",			Token::ADDVAR},
				{"again",		Token::AGAIN},			{"and",			Token::AND},			{"andv",		Token::ANDV},		{"collided?",	Token::COLLIDEDQ},
				{"col?",		Token::COLLIDEDQ},		{"delay",		Token::DELAY},			{"destroy",		Token::DESTROY},	{"destroyed?",	Token::DESTROYEDQ},
				{"else",		Token::ELSE},			{"endgame",		Token::ENDGAME},		{"endif",		Token::ENDIF},		{"end",			Token::END},
				{"execute",		Token::EXECUTE},		{"ex",			Token::EXECUTE},		{"goto",		Token::GOTO},		{"if",			Token::IF},
				{"include",		Token::INCLUDE},		{"invis?",		Token::INVISQ},			{"invis",		Token::INVIS},		{"iv",			Token::INVIS},
				{"loop",		Token::LOOP},			{"mode",		Token::MODE},			{"moveto",		Token::MOVETO},		{"move",		Token::MOVE},
				{"notv",		Token::NOTV},			{"orv",			Token::ORV},			{"or",			Token::OR},			{"getxpos",		Token::GETXPOS},
				{"getypos",		Token::GETYPOS},		{"getzpos",		Token::GETZPOS},		{"print",		Token::PRINT},		{"restart",		Token::RESTART},
				{"redraw",		Token::REDRAW},			{"remove",		Token::REMOVE},			{"sound",		Token::SOUND},		{"setvar",		Token::SETVAR},
				{"shot?",		Token::SHOTQ},			{"startanim",	Token::STARTANIM},		{"start",		Token::START},		{"stopanim",	Token::STOPANIM},
				{"subvar",		Token::SUBVAR},			{"sub",			Token::SUBVAR},			{"syncsnd",		Token::SYNCSND},	{"then",		Token::THEN},
				{"timer?",		Token::TIMERQ},			{"togvis",		Token::TOGVIS},			{"tog",			Token::TOGVIS},		{"triganim",	Token::TRIGANIM},
				{"updatei",		Token::UPDATEI},		{"var=?",		Token::VAREQ},			{"var>?",		Token::VARGQ},		{"var<?",		Token::VARLQ},
				{"v=?",			Token::VAREQ},			{"v>?",			Token::VARGQ},			{"v<?",			Token::VARLQ},		{"vis?",		Token::VISQ},
				{"vis",			Token::VIS},			{"waittrig",	Token::WAITTRIG},		{"wait",		Token::WAIT},		{",",			Token::COMMA},
				{"(",			Token::OPENBRACKET},	{")",			Token::CLOSEBRACKET},

				// we'll use Name = NULL as a list terminator
				{NULL,			Token::UNKNOWN}
			};

			// there's nothing so clever as a hash lookup; we'll do a linear
			// search on the table — that's helpful because it means we will
			// correctly consume longer reserved words before their shorter
			// fragments (eg, addvar versus add)
			TokenMapping *currentToken = Tokens;
			while(currentToken->name)
			{
				// check if the text matches
				if(!SourceCode->compare(CodePointer, strlen(currentToken->name), currentToken->name))
				{
					LastToken = new Token(currentToken->type);
					CodePointer += strlen(currentToken->name);
					return LastToken;
				}
				currentToken++;
			}

			// if we get to here then we didn't find the token in the lookup table
			// so it's time to try some other guesses...

			// maybe we've found a string literal? If so it'll be bounded by double quotes
			if((*SourceCode)[CodePointer] == '"')
			{
				// skip the initial quote
				CodePointer++;

				// find the end
				while((*SourceCode)[CodePointer] != '"') CodePointer++;

				// create a token
				LastToken = new Token(new std::string(*SourceCode, CodePointerBeforeToken+1, CodePointer - CodePointerBeforeToken - 2));
				return LastToken;
			}

			// maybe it's a reference to a constant or a variable?
			// constants are integer numbers; variables look like
			// v0, v1, etc

			// assume we've found a constant, if anything; change that
			// to having found a variable reference if it starts with a v,
			// advancing the code pointer at the same time
			Token::Type discoveredType = Token::CONSTANT;
			if((*SourceCode)[CodePointer] == 'v')
			{
				discoveredType = Token::VARIABLE;
				CodePointer++;
			}

			// read in an integer number
			const char *startOfNumber = &SourceCode->c_str()[CodePointer];
			char *endOfNumber;
			long value = strtol(startOfNumber, &endOfNumber, 10);
			if(startOfNumber != endOfNumber)
			{
				LastToken = new Token(discoveredType, (uint32_t)value);
				return LastToken;
			}

			// okay, fine, we've got no idea what we found
			LastToken = new Token(Token::UNKNOWN);
			return LastToken;
		}

		// Expect takes an expected token type and raises an error
		// condition if a different kind of token is found
		void Expect(Token::Type type)
		{
			Token *newToken = GetToken();
			if(newToken->getType() != type)
				errorNumber = 1;

			delete newToken;
		}

		// functions take up to three arguments, and function calls
		// look like C with an opening and closing bracket and commas
		// separating arguments
		void GetArguments(FCLInstruction *instruction)
		{
			Token *tokens[3];

			Expect(Token::OPENBRACKET);
			unsigned int tokenPointer;
			for(tokenPointer = 0; tokenPointer < 3; tokenPointer++)
			{
				tokens[tokenPointer] = GetToken();

				Token *nextToken = GetToken();
				Token::Type type = nextToken->getType();
				delete nextToken;

				if(type == Token::CLOSEBRACKET) break;
			}

			switch(tokenPointer)
			{
				case 0: instruction->setArguments(tokens[0]);						break;
				case 1: instruction->setArguments(tokens[0], tokens[1]);			break;
				case 2: instruction->setArguments(tokens[0], tokens[1], tokens[2]);	break;
				default: break;
			}
		}

	public:
		CSyntaxBuilder(std::string *SourceCode);
		
		std::vector<FCLInstruction *> *getInstructions(bool isSubBranch)
		{
			std::vector<FCLInstruction *> *instructions = new std::vector<FCLInstruction *>;

			while(1)
			{
				// get the next token
				Token *newToken = GetToken();

				// if we've hit end of file then stop
				if(newToken->getType() == Token::ENDOFFILE)
				{
					delete newToken;
					break;
				}

				// create an instruction to hold this token
				FCLInstruction *instruction = new FCLInstruction(newToken->getType());
				instructions->push_back(instruction);

				// determine whether we need to get arguments; a remnant of
				// an older implementation means that I've divided the various functions
				// by number of arguments — that can be fixed once the code as a whole
				// is working properly again
				switch(newToken->getType())
				{
					/*
						functions with no arguments
					*/
					case Token::SWAPJET:
					case Token::REDRAW:		case Token::END:		case Token::ENDGAME:	case Token::ACTIVATEDQ:
					case Token::COLLIDEDQ:	case Token::TIMERQ:		case Token::AND:		case Token::OR:
					case Token::WAIT:		case Token::SHOTQ:		case Token::IF:			case Token::RESTART:
					case Token::START:		case Token::WAITTRIG:	case Token::AGAIN:
					break;

					/*
						functions that are always unary
					*/
					case Token::SETBIT:		case Token::CLEARBIT:		case Token::TOGGLEBIT:
					case Token::LOOP:		case Token::UPDATEI:		case Token::DELAY:			case Token::EXECUTE:
					case Token::INCLUDE:	case Token::MODE:			case Token::SOUND:			case Token::SYNCSND:
					case Token::TRIGANIM:	case Token::REMOVE:

					/*
						functions that are always binary
					*/
					case Token::BITNOTEQ:	case Token::VARNOTEQ:
					case Token::VAREQ:		case Token::VARLQ:			case Token::VARGQ:			case Token::SETVAR:
					case Token::SUBVAR:		case Token::ADDVAR:			case Token::PRINT:			case Token::ANDV:
					case Token::ORV:		case Token::NOTV:

					/*
						functions that may be binary or unary
					*/
					case Token::INVIS:		case Token::VIS:			case Token::GOTO:			case Token::DESTROY:
					case Token::TOGVIS:		case Token::DESTROYEDQ:		case Token::VISQ:			case Token::INVISQ:
					case Token::STARTANIM:	case Token::STOPANIM:

					/*
						explicitly ternary functions
					*/
					case Token::MOVE:		case Token::MOVETO:

					/*
						functions that may be binary or ternary
					*/
					case Token::GETXPOS:	case Token::GETYPOS:		case Token::GETZPOS:
						GetArguments(instruction);
					break;

					/*
						conditionality is the only thing that can make an actual tree
						of the syntax; we grab the stream of instructions that follows
						and stuff it into the instruction as passed or failed branch
					*/
					case Token::THEN:
					{
						std::vector<FCLInstruction *> *thenInstructions = getInstructions(true);
						std::vector<FCLInstruction *> *elseInstructions = NULL;

						// check for an else branch
						Token *nextToken = GetToken();
						if(nextToken->getType() == Token::ELSE)
						{
							elseInstructions = getInstructions(true);
							delete nextToken;
						}
						else
							UnGetToken();

						instruction->setBranches(thenInstructions, elseInstructions);
					}
					break;

					/*
						technically ENDIF and ELSE should be encountered only if we're
						parsing a sub branch, but the Construction Kit seems silently to
						accept ENDIF and ELSE at the top level, hence the argument
					*/
					case Token::ENDIF:
						if(isSubBranch) return instructions;
					break;

					case Token::ELSE:
						if(isSubBranch)
						{
							UnGetToken();
							return instructions;
						}
					break;

					/*
						for now, output an error to stderr if a problem was encountered;
						something more robust will be needed at some point
					*/
					default:
						std::cerr << "don't know how to handle token before [" << CodePointer << "]" << std::endl;
						std::cerr << SourceCode << std::endl;
						return instructions;
					break;
				}
			}

			return instructions;
		}

};

std::vector<FCLInstruction *> *getInstructions(std::string *sourceCode)
{
	CSyntaxBuilder syntaxBuilder(sourceCode);
	return syntaxBuilder.getInstructions(false);
}
