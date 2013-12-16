//
//  16bitDetokeniser.cpp
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "16bitDetokeniser.h"
#include <sstream>

shared_ptr<string> detokenise16bitCondition(vector <uint8_t> &tokenisedCondition)
{
	stringstream detokenisedStream;
	vector<uint8_t>::size_type bytePointer = 0;
	vector<uint8_t>::size_type sizeOfTokenisedContent = tokenisedCondition.size();

	while(bytePointer < sizeOfTokenisedContent-1)
	{
		// byte 1 = number of arguments, byte 2 = opcode
		uint8_t numberOfArguments	= tokenisedCondition[bytePointer];
		uint8_t opcode				= tokenisedCondition[bytePointer+1];
		bytePointer += 2;

		// make sure we have enough buffer left to read all the arguments
		if(bytePointer + numberOfArguments*2 > sizeOfTokenisedContent) break;

		// write out the operation
		switch(opcode)
		{
			case 0x01:	detokenisedStream << "ACTIVATED? ";		break;
			case 0x02:	detokenisedStream << "COLLIDED? ";		break;
			case 0x03:	detokenisedStream << "SHOT? ";			break;
			case 0x04:	detokenisedStream << "TIMER? ";			break;

			case 0x10:	detokenisedStream << "SETVAR ";			break;
			case 0x11:	detokenisedStream << "ADDVAR ";			break;
			case 0x12:	detokenisedStream << "SUBVAR ";			break;

			case 0x16:	detokenisedStream << "VAR=? ";			break;
			case 0x17:	detokenisedStream << "VAR>? ";			break;
			case 0x18:	detokenisedStream << "VAR<? ";			break;

			case 0x30:	detokenisedStream << "INVIS ";			break;
			case 0x31:	detokenisedStream << "VIS ";			break;
			case 0x32:	detokenisedStream << "TOGVIS ";			break;
			case 0x33:	detokenisedStream << "DESTROY ";		break;

			case 0x34:	detokenisedStream << "INVIS? ";			break;
			case 0x35:	detokenisedStream << "VIS? ";			break;
			
			case 0x36:	detokenisedStream << "MOVE ";			break;
			case 0x3a:	detokenisedStream << "MOVETO ";			break;

			case 0x40:	detokenisedStream << "IF ";				break;
			case 0x41:	detokenisedStream << "THEN ";			break;
			case 0x42:	detokenisedStream << "ELSE ";			break;
			case 0x43:	detokenisedStream << "ENDIF ";			break;
			case 0x44:	detokenisedStream << "AND ";			break;
			case 0x45:	detokenisedStream << "OR ";				break;

			case 0x50:	detokenisedStream << "STARTANIM ";		break;
			case 0x51:	detokenisedStream << "STOPANIM ";		break;

			case 0x52:	detokenisedStream << "START ";			break;
			case 0x53:	detokenisedStream << "RESTART ";		break;

			case 0x54:	detokenisedStream << "INCLUDE ";		break;

			case 0x55:	detokenisedStream << "WAITTRIG ";		break;
			case 0x56:	detokenisedStream << "TRIGANIM ";		break;

			case 0x60:	detokenisedStream << "LOOP ";			break;
			case 0x61:	detokenisedStream << "AGAIN ";			break;

			case 0x70:	detokenisedStream << "SOUND ";			break;
			case 0x71:	detokenisedStream << "SYNCSND ";		break;

			case 0x80:	detokenisedStream << "WAIT ";			break;
			case 0x81:	detokenisedStream << "DELAY ";			break;

			case 0x82:	detokenisedStream << "UPDATEI ";		break;
			case 0x83:	detokenisedStream << "PRINT ";			break;
			case 0x84:	detokenisedStream << "REDRAW ";			break;

			case 0x85:	detokenisedStream << "MODE ";			break;
			case 0x86:	detokenisedStream << "ENDGAME ";		break;

			case 0x87:	detokenisedStream << "EXECUTE ";		break;
			case 0x90:	detokenisedStream << "GOTO ";			break;

			case 0xff:	detokenisedStream << "END ";			break;

			default:
				detokenisedStream << "<UNKNOWN: " << std::hex << (int)opcode << "> " << std::dec;
			break;
		}

		// PRINT is a special case, requiring us to grab a string,
		// but everything else is uniform
		if(numberOfArguments)
		{
			// arguments are enclosed in brackets
			detokenisedStream << "(";

			if(opcode == 0x83)
			{
				// the first argument is a string, which is encoded as
				// a two-byte string length (in big endian form)
				uint16_t stringLength =
					(uint16_t)(
						(tokenisedCondition[bytePointer] << 8) |
						tokenisedCondition[bytePointer+1]
					);
				bytePointer += 2;
				numberOfArguments--;

				detokenisedStream << "\"";
				for(uint16_t stringPosition = 0; stringPosition < stringLength; stringPosition++)
				{
					char nextChar = (char)tokenisedCondition[bytePointer + stringPosition];

					// TODO: spot special characters here

					if(nextChar)
						detokenisedStream << nextChar;
				}
				detokenisedStream << "\"";
				bytePointer += stringLength;

				// strings are rounded up in length to the end
				// of this two-byte quantity
				if(stringLength&1) bytePointer++;
				numberOfArguments -= (stringLength+1) >> 1;

				// that should leave an argument, but you can't be too safe
				if(numberOfArguments) detokenisedStream << ", ";
			}

			for(uint8_t argumentNumber = 0; argumentNumber < numberOfArguments; argumentNumber++)
			{
				// each argument is encoded in two bytes...
				uint8_t indexHighByte		= tokenisedCondition[bytePointer];
				uint8_t indexLowByte		= tokenisedCondition[bytePointer+1];
				bytePointer += 2;

				// if the high bit of the first byte is clear then this
				// argument is a constant; if it's set then this is a variable
				if(indexHighByte&0x80) detokenisedStream << "V";
				indexHighByte &= 0x7f;

				// the second byte is either the constant or the index of the
				// variable
				detokenisedStream << (int)(indexLowByte | (indexHighByte << 8));

				// ... and arguments are separated by commas, of course
				if(argumentNumber+1 < numberOfArguments)
					detokenisedStream << ", ";
			}

			// add a closing bracket
			detokenisedStream << ")";
		}

		detokenisedStream << endl;
	}

	shared_ptr<string> outputString(new string);
	*outputString = detokenisedStream.str();

	return outputString;
}
