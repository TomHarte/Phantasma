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

#include "Object.h"

#include <map>

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

		vector<uint8_t>::size_type getFileOffset()
		{
			return bytePointer;
		}

		void setFileOffset(vector<uint8_t>::size_type newOffset)
		{
			bytePointer = newOffset;
		}
};

static shared_ptr<Object> loadObject(StreamLoader &stream)
{
	// get object flags and type
	uint8_t objectFlags = stream.get8();
	Object::Type objectType = (Object::Type)stream.get8();

	// get unknown value
	uint16_t skippedShort = stream.get16();

	// grab location, size
	Vector3d position, size;
	position.x		= stream.get16();
	position.y		= stream.get16();
	position.z		= stream.get16();
	size.x			= stream.get16();
	size.y			= stream.get16();
	size.z			= stream.get16();

	// object ID
	uint16_t objectID = stream.get16();

	// size of object on disk; we've accounted for 20 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint32_t byteSizeOfObject = (uint32_t)(stream.get16() << 1) - 20;

	shared_ptr<Object> returnObject;
	std::cout << "Object " << objectID << "; type " << (int)objectType << std::endl;

	switch(objectType)
	{
		default:
		{
			// read the appropriate number of colours
			int numberOfColours = GeometricObject::numberOfColoursForObjectOfType(objectType);
			std::vector<uint8_t> *colours = new std::vector<uint8_t>;
			for(uint8_t colour = 0; colour < numberOfColours; colour++)
			{
				colours->push_back(stream.get8());
				byteSizeOfObject--;
			}

			// read extra vertices if required...
			int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
			std::vector<uint16_t> *ordinates = NULL;

			if(numberOfOrdinates)
			{
				ordinates = new std::vector<uint16_t>;

				for(int ordinate = 0; ordinate < numberOfOrdinates; ordinate++)
				{
					ordinates->push_back(stream.get16());
					byteSizeOfObject -= 2;
				}
			}

			// grab the object condition, if there is one
			FCLInstructionVector instructions;
			if(byteSizeOfObject)
			{
				shared_ptr<vector<uint8_t>> conditionData = stream.nextBytes(byteSizeOfObject);

				shared_ptr<string> conditionSource = detokenise16bitCondition(*conditionData);
				instructions = getInstructions(conditionSource.get());
			}
			byteSizeOfObject = 0;

			// create an object
			returnObject = shared_ptr<Object>(
				new GeometricObject(
					objectType,
					objectID,
					position,
					size,
					colours,
					ordinates,
					instructions));
		}
		break;

		case Object::Entrance:
		case Object::Sensor:
		case Object::Group:
		break;
	}

	// skip whatever we didn't understand
	stream.skipBytes(byteSizeOfObject);

	return returnObject;
}

static void loadArea(StreamLoader &stream)
{
	// the lowest bit of this value seems to indicate
	// horizon on or off; this is as much as I currently know
	uint16_t skippedValue		= stream.get16();
	uint16_t numberOfObjects	= stream.get16();
	uint16_t areaNumber			= stream.get16();

	cout << "Area " << areaNumber << endl;
	cout << "Skipped value " << skippedValue << endl;
	cout << "Objects: " << numberOfObjects << endl;

	// I've yet to decipher this fully
	uint16_t horizonColour	= stream.get16();
	cout << "Horizon colour " << hex << (int)horizonColour << dec << endl;

	// this is just a complete guess
	for(int paletteEntry = 0; paletteEntry < 22; paletteEntry++)
	{
		uint8_t paletteColour		= stream.get8();
		cout << "Palette colour (?) " << hex << (int)paletteColour << dec << endl;
	}

	// we'll need to collate all objects and entrances; it's likely a
	// plain C array would do but maps are safer and the total application
	// cost is going to be negligible regardless
	std::map<uint16_t, shared_ptr<Object>> objectsByID;
	std::map<uint16_t, shared_ptr<Object>> entrancesByID;

	// get the objects or whatever; entrances use a unique numbering
	// system and have the high bit of their IDs set in the original file
	for(uint16_t object = 0; object < numberOfObjects; object++)
	{
		shared_ptr<Object> newObject = loadObject(stream);

		if(newObject.get())
		{
			if(newObject->getType() == Object::Entrance)
			{
				entrancesByID[newObject->getObjectID() & 0x7fff] = newObject;
			}
			else
			{
				objectsByID[newObject->getObjectID()] = newObject;
			}
		}
	}

	cout << objectsByID.size() << " Objects" << endl;
	cout << entrancesByID.size() << " Entrances" << endl;
}

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

	// this brings us to the beginning of the embedded
	// .KIT file, so we'll grab the base offset for
	// finding areas later
	vector<uint8_t>::size_type baseOffset = streamLoader.getFileOffset();

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

	// grab the areas (well, for now, print them)
	for(uint16_t area = 0; area < numberOfAreas; area++)
	{
//		cout << "Area " << area+1 << endl;

		streamLoader.setFileOffset(fileOffsetForArea[area] + baseOffset);
		loadArea(streamLoader);

		cout << endl;
	}

	delete[] fileOffsetForArea;
	return true;
}
