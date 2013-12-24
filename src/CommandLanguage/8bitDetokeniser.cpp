//
//  8bitDetokeniser.cpp
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

/*
	This has been implemented based on John Elliott's 2001
	reverse engineering of Driller; see http://www.seasip.demon.co.uk/ZX/Driller/
*/

#include "8bitDetokeniser.h"
#include <sstream>

static const int k8bitVariableShield	= 256;
static const int k8bitVariableEnergy	= 257;
static const int k8bitVariableScore		= 258;

shared_ptr<string> detokenise8bitCondition(vector <uint8_t> &tokenisedCondition)
{
	stringstream detokenisedStream;
	vector<uint8_t>::size_type bytePointer = 0;
	vector<uint8_t>::size_type sizeOfTokenisedContent = tokenisedCondition.size();

	// on the 8bit platforms, all instructions have a conditional flag;
	// we'll want to convert them into runs of "if shot? then" and "if collided? then",
	// and we'll want to start that from the top
	uint8_t conditionalIsShot = 0x1;

	// this lookup table tells us how many argument bytes to read per opcode
	uint8_t argumentsRequiredByOpcode[32] =
	{
		0,	3,	1,	1,	1,	1,	2,	2,
		2,	1,	1,	2,	1,	1,	2,	1,
		1,	2,	2,	1,	2,	0,	0,	0,
		0,	1,	0,	1,	1,	1,	1,	1
	};

	while(bytePointer < sizeOfTokenisedContent)
	{
		// get the conditional type of the next operation
		uint8_t newConditionalIsShot = tokenisedCondition[bytePointer] & 0x80;

		// if the conditional type has changed then end the old conditional,
		// if we were in one, and begin a new one
		if(newConditionalIsShot != conditionalIsShot)
		{
			conditionalIsShot = newConditionalIsShot;
			if(bytePointer) detokenisedStream << "ENDIF" << endl;

			if(conditionalIsShot)
				detokenisedStream << "IF SHOT? THEN" << endl;
			else
				detokenisedStream << "IF COLLIDED? THEN" << endl;
		}

		// get the actual operation
		uint8_t opcode = tokenisedCondition[bytePointer] & 0x1f;
		bytePointer++;

		// figure out how many argument bytes we're going to need,
		// check we have enough bytes left to read
		uint8_t numberOfArguments = argumentsRequiredByOpcode[opcode];
		if(bytePointer + numberOfArguments > sizeOfTokenisedContent)
			break;

		// generate the string
		switch(opcode)
		{
			default:
				detokenisedStream << "<UNKNOWN 8 bit: " << std::hex << (int)opcode << "> " << std::dec;
			break;

			case 0: break;			// NOP
			case 1:					// add three-byte value to score
			{
				int32_t additionValue =
					tokenisedCondition[bytePointer] |
					(tokenisedCondition[bytePointer+1] << 8) |
					(tokenisedCondition[bytePointer+2] << 16);
				detokenisedStream << "ADDVAR (" << additionValue << ", v" << k8bitVariableScore << ")";
				bytePointer += 3;
				numberOfArguments = 0;
			}
			break;
			case 2:					// add one-byte value to energy
				detokenisedStream << "ADDVAR (" << (int)tokenisedCondition[bytePointer] << ", v" << k8bitVariableEnergy << ")";
				bytePointer ++;
				numberOfArguments = 0;
			break;
			case 19:				// add one-byte value to shield
				detokenisedStream << "ADDVAR (" << (int)tokenisedCondition[bytePointer] << ", v" << k8bitVariableShield << ")";
				bytePointer ++;
				numberOfArguments = 0;
			break;

			case 6:		case 3:		detokenisedStream << "TOGVIS (";		break;	// these all come in unary and binary versions,
			case 7:		case 4:		detokenisedStream << "VIS (";			break;	// hence each getting two case statement entries
			case 8:		case 5:		detokenisedStream << "INVIS (";			break;

			case 9:					detokenisedStream << "ADDVAR (1, v";	break;
			case 10:				detokenisedStream << "SUBVAR (1, v";	break;

			case 11:	// end condition if a variable doesn't have a particular value
				detokenisedStream	<< "IF VAR!=? (v" << (int)tokenisedCondition[bytePointer] << ", " << (int)tokenisedCondition[bytePointer+1] << ") "
									<< "THEN END ENDIF";
				bytePointer += 2;
				numberOfArguments = 0;
			break;
			case 14:	// end condition if a bit doesn't have a particular value
				detokenisedStream	<< "IF BIT!=? (" << (int)tokenisedCondition[bytePointer] << ", " << (int)tokenisedCondition[bytePointer+1] << ") "
									<< "THEN END ENDIF";
				bytePointer += 2;
				numberOfArguments = 0;
			break;
			case 30:	// end condition if an object is invisible
				detokenisedStream	<< "IF INVIS? (" << (int)tokenisedCondition[bytePointer] << ") "
									<< "THEN END ENDIF";
				bytePointer ++;
				numberOfArguments = 0;
			break;
			case 31:	// end condition if an object is visible
				detokenisedStream	<< "IF VIS? (" << (int)tokenisedCondition[bytePointer] << ") "
									<< "THEN END ENDIF";
				bytePointer ++;
				numberOfArguments = 0;
			break;

			case 12:				detokenisedStream << "SETBIT (";		break;
			case 13:				detokenisedStream << "CLRBIT (";		break;

			case 15:				detokenisedStream << "SOUND (";			break;
			case 17:	case 16:	detokenisedStream << "DESTROY (";		break;
			case 18:				detokenisedStream << "GOTO (";			break;

			case 21:				detokenisedStream << "SWAPJET";			break;
			case 26:				detokenisedStream << "REDRAW";			break;
			case 27:				detokenisedStream << "DELAY (";			break;
			case 28:				detokenisedStream << "SYNCSND (";		break;
			case 29:				detokenisedStream << "TOGBIT (";		break;

			case 25:
			{
				// this should toggle border colour; it's therefore a no-op
				bytePointer++;
				numberOfArguments = 0;
			}
			break;

			case 20:
				detokenisedStream	<< "SETVAR (" << (int)tokenisedCondition[bytePointer] << ", v" << (int)tokenisedCondition[bytePointer+1] << ") ";
				bytePointer += 2;
				numberOfArguments = 0;
			break;
		}

		// if there are any regular arguments to add, do so
		if(numberOfArguments)
		{
			for(uint8_t argumentNumber = 0; argumentNumber < numberOfArguments; argumentNumber++)
			{
				detokenisedStream << (int)tokenisedCondition[bytePointer];
				bytePointer++;

				if(argumentNumber < numberOfArguments-1)
					detokenisedStream << ", ";
			}

			detokenisedStream << ")";
		}

		// throw in a newline
		detokenisedStream << endl;
	}

	shared_ptr<string> outputString(new string);
	*outputString = detokenisedStream.str();

	return outputString;

}
