//
//  16bitBinaryLoader.cpp
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "16bitBinaryLoader.h"
#include "Parser.h"
#include "16bitDetokeniser.h"

class StreamLoader
{
	private:
		vector<uint8_t>::size_type bytePointer;
		vector <uint8_t> binary;

	public:
		StreamLoader(vector <uint8_t> &_binary)
		{
			binary = _binary;
			bytePointer = 0;
		}

		uint8_t get8()
		{
			if(!eof())
				return binary[bytePointer++];

			return 0;
		}

		uint16_t get16()
		{
			uint16_t result = (uint16_t)(get8() << 8);
			result |= get8();
			return result;
		}

		uint32_t get32()
		{
			uint32_t result = (uint32_t)(get16() << 16);
			result |= get16();
			return result;
		}

		bool eof()
		{
			return bytePointer >= binary.size();
		}

		void alignPointer()
		{
			if(bytePointer&1) bytePointer++;
		}

		void skipBytes(vector<uint8_t>::size_type numberOfBytes)
		{
			bytePointer += numberOfBytes;
		}
		
		shared_ptr<vector<uint8_t>> nextBytes(vector<uint8_t>::size_type numberOfBytes)
		{
			shared_ptr<vector<uint8_t>> returnBuffer(new vector<uint8_t>);

			while(numberOfBytes--)
				returnBuffer->push_back(get8());

			return returnBuffer;
		}
};

bool load16bitBinary(vector <uint8_t> &binary)
{
	StreamLoader streamLoader(binary);

	// find DOS end of file and consume it
	while(!streamLoader.eof() && streamLoader.get8() != 0x1a);
	streamLoader.get8();

	// advance to the next two-byte boundary if necessary
	streamLoader.alignPointer();

	// skip bytes with meaning unknown
	streamLoader.get16();

	// check that the next two bytes are "PC", then
	// skip the number that comes after
	if(streamLoader.get8() != 'C' || streamLoader.get8() != 'P') return false;
	streamLoader.get16();

	// start grabbing some of the basics...

	uint16_t numberOfAreas = streamLoader.get16();
	streamLoader.get16();							// meaning unknown

	cout << numberOfAreas << " Areas" << endl;

	uint16_t windowCentreX	= streamLoader.get16();
	uint16_t windowCentreY	= streamLoader.get16();
	uint16_t windowWidth	= streamLoader.get16();
	uint16_t windowHeight	= streamLoader.get16();

	cout << "Window centre: ("	<< windowCentreX	<< ", " << windowCentreY	<< ")" << endl;
	cout << "Window size: ("	<< windowWidth		<< ", " << windowHeight		<< ")" << endl;

	uint16_t scaleX	= streamLoader.get16();
	uint16_t scaleY	= streamLoader.get16();
	uint16_t scaleZ	= streamLoader.get16();

	cout << "Scale: " << scaleX << ", " << scaleY << ", " << scaleZ << endl;

	uint16_t timerReload	= streamLoader.get16();

	cout << "Timer: every " << timerReload << " 50Hz frames";

	uint16_t maximumActivationDistance	= streamLoader.get16();
	uint16_t maximumFallDistance		= streamLoader.get16();
	uint16_t maximumClimbDistance		= streamLoader.get16();

	cout << "Maximum activation distance: "		<< maximumActivationDistance << endl;
	cout << "Maximum fall distance: "			<< maximumFallDistance << endl;
	cout << "Maximum climb distance: "			<< maximumClimbDistance << endl;

	uint16_t startArea					= streamLoader.get16();
	uint16_t startEntrance				= streamLoader.get16();

	cout << "Start at entrance " << startEntrance << " in area " << startArea << endl;

	uint16_t playerHeight				= streamLoader.get16();
	uint16_t playerStep					= streamLoader.get16();
	uint16_t playerAngle				= streamLoader.get16();

	cout << "Height " << playerHeight << ", stap " << playerStep << ", angle " << playerAngle << endl;

	uint16_t startVehicle				= streamLoader.get16();
	uint16_t executeGlobalCondition		= streamLoader.get16();

	cout << "Start vehicle " << startVehicle << ", execute global condition " << executeGlobalCondition << endl;

	// I haven't figured out what the next 106
	// bytes mean, so we'll skip them — global objects
	// maybe? Likely not 106 bytes in every file.
	//
	// ADDED: having rediscovered my source for the 8bit
	// file format, could this be shading information by
	// analogy with that?
	streamLoader.skipBytes(106);

	// at this point I should properly load the border/key/mouse
	// bindings, but I'm not worried about it for now.
	//
	// Format is:
	//		(left x, top y, right x, bottom y) - all 16 bit
	//		keyboard key as an ASCII character (or zero for none)
	//		mouse button masl; 00 = both, 01 = left, 02 = right
	//
	// So, 10 bytes per binding. Bindings are listed in the order:
	//
	//	move forwards, move backwards, move right, move left, rise,
	//	fall, turn left, turn right, look up, look down, tilt left,
	//	tilt right, face forward, u-turn, change vehicle type,
	//	select this vehicle, quit game, fire, activate object,
	//	centre cursor on/off, save game position, load game position
	//
	// So 35 total. Which means this area takes up 350 bytes.
	streamLoader.skipBytes(350);

	// there are then file pointers for every area — these are
	// the number of shorts from the 'PC' tag, so multiply by
	// two for bytes. Each is four bytes
	uint32_t *fileOffsetForArea = new uint32_t[numberOfAreas];
	for(uint16_t area = 0; area < numberOfAreas; area++)
		fileOffsetForArea[area] = (uint32_t)streamLoader.get32() << 1;

	// now come the global conditions
	uint16_t numberOfGlobalConditions = streamLoader.get16();
	for(uint16_t globalCondition = 0; globalCondition < numberOfGlobalConditions; globalCondition++)
	{
		// 12 bytes for the name of the condition;
		// we don't care
		streamLoader.skipBytes(12);

		// get the length and the data itself, converting from
		// shorts to bytes
		uint32_t lengthOfCondition = (uint32_t)streamLoader.get16() << 1;

		// get the condition
		shared_ptr<vector<uint8_t>> conditionData = streamLoader.nextBytes(lengthOfCondition);
		
		cout << "Global condition " << globalCondition+1 << endl;
		cout << *detokenise16bitCondition(*conditionData) << endl;
	}

	delete[] fileOffsetForArea;
	return true;
}
