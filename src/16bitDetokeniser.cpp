//
//  16bitDetokeniser.cpp
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "16bitDetokeniser.h"

shared_ptr<string> detokenise16bitCondition(vector <uint8_t> &tokenisedCondition)
{
	shared_ptr<string> newString(new string);
	std::vector<uint8_t>::size_type bytePointer = 0;
	std::vector<uint8_t>::size_type sizeOfTokenisedContent = tokenisedCondition.size();

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
			case 0x10:	*newString += "SETVAR ";		break;
			case 0x11:	*newString += "ADDVAR ";		break;
			case 0x12:	*newString += "SUBVAR ";		break;

			case 0x16:	*newString += "VAR=? ";			break;
			case 0x17:	*newString += "VAR>? ";			break;

			case 0x40:	*newString += "IF ";			break;
			case 0x41:	*newString += "THEN ";			break;
			case 0x42:	*newString += "ELSE ";			break;
			case 0x43:	*newString += "ENDIF ";			break;

			case 0x50:	*newString += "STARTANIM ";		break;

			case 0x81:	*newString += "DELAY ";			break;
			case 0x82:	*newString += "UPDATEI ";		break;
			case 0x83:	*newString += "PRINT ";			break;
			case 0x84:	*newString += "REDRAW ";		break;
			
			case 0x90:	*newString += "GOTO ";			break;
			
			default:
				*newString += "<UNKNOWN> ";
				cerr << "Unknown opcode: " << opcode;
			break;
		}

		// PRINT is a special case, requiring us to grab a string,
		// but everything else is uniform
		if(numberOfArguments)
		{
			// arguments are enclosed in brackets
			*newString += "(";

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

				*newString += "\"";
				for(uint16_t stringPosition = 0; stringPosition < stringLength; stringPosition++)
				{
					char nextChar = (char)tokenisedCondition[bytePointer + stringPosition];

					// TODO: spot special characters here

					if(nextChar)
						newString->append(1, nextChar);
				}
				*newString += "\"";
				bytePointer += stringLength;
				numberOfArguments -= stringLength >> 1;

				// that should leave an argument, but you can't be too safe
				if(numberOfArguments) *newString += ", ";
			}

			for(uint8_t argumentNumber = 0; argumentNumber < numberOfArguments; argumentNumber++)
			{
				// each argument is encoded in two bytes...
				uint8_t isVariableMarker	= tokenisedCondition[bytePointer];
				uint8_t index				= tokenisedCondition[bytePointer+1];
				bytePointer += 2;

				// if the first byte is zero then this argument is a constant;
				// if it's 0x80 then this argument is a variable reference
				if(isVariableMarker) *newString += "v";

				// the second byte is either the constant or the index of the
				// variable
				char indexString[4];
				snprintf(indexString, sizeof(indexString), "%d", index);
				*newString += indexString;

				// ... and arguments are separated by commas, of course
				if(argumentNumber < numberOfArguments)
					*newString += ", ";
			}

			// add a closing bracket
			*newString += ")";
		}

		*newString += "\n";
	}

	return newString;
}
