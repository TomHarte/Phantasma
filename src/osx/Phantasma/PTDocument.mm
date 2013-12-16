//
//  PTDocument.m
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#import "PTDocument.h"
#include "Parser.h"
#include "16bitDetokeniser.h"

@implementation PTDocument

- (id)init
{
    self = [super init];
    if (self) {
		// Add your subclass-specific initialization here.
    }
    return self;
}

- (NSString *)windowNibName
{
	// Override returning the nib file name of the document
	// If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
	return @"PTDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
	[super windowControllerDidLoadNib:aController];
	// Add any code here that needs to be executed once the windowController has loaded the document's window.
}

+ (BOOL)autosavesInPlace
{
    return YES;
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
	// Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
	// You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
	NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
	@throw exception;
	return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
	// I'm still working the full format of .RUN files;
	// for right now I'm going to hard code in the ranges
	// of some conditions and use those to advance the 16-bit detokeniser
	NSRange conditionRanges[] =
	{
		// global conditions
/*		{.location = 658, .length = 158},
		{.location = 830, .length = 80},
		{.location = 924, .length = 124},
		{.location = 1062, .length = 68},
		{.location = 1144, .length = 78},
		{.location = 1236, .length = 82},
		{.location = 1332, .length = 460},
		{.location = 1806, .length = 48},
		{.location = 1868, .length = 182},

		// area 1 condition
		{.location = 4988, .length = 72},

		// area 1, object 12 condition
		{.location = 2620, .length = 12},	// observation: this makes it look like 26 bytes
											// fixed per cuboid + condition length
		{.location = 2658, .length = 40},
		{.location = 2720, .length = 14},
		
		// ...

		{.location = 2854, .length = 20},
		{.location = 3450, .length = 28},
		{.location = 3988, .length = 72},
		{.location = 4396, .length = 72},
		{.location = 8118, .length = 60},

		// some animators
		{.location = 23410, .length = 94},
		{.location = 26786, .length = 44},

		{.location = 7838, .length = 100},
		{.location = 28488, .length = 916},
		{.location = 10642, .length = 38},
		{.location = 21554, .length = 230},
		{.location = 20746, .length = 174},
		{.location = 23354, .length = 30},*/

		{.location = 576, .length = 178},


		{.location = NSNotFound, .length = 0},
	};

	NSRange *testRange = conditionRanges;
	while(testRange->location != NSNotFound)
	{
		NSData *conditionData = [data subdataWithRange:*testRange];

		// one suspects there's an easier way to fill a std::vector
		// from an NSData than this, but I'm a C++ novice
		std::vector<uint8_t> conditionDataVector;
		const uint8_t *bytes = (uint8_t *)[conditionData bytes];
		for(NSUInteger index = 0; index < [conditionData length]; index++)
		{
			conditionDataVector.push_back(bytes[index]);
		}

		NSLog(@"Condition %02ld", (testRange - conditionRanges)+1);
		cout << *detokenise16bitCondition(conditionDataVector);

		testRange++;
	}

/*	const char *sampleProgram =
		"         IF				"
		"         VAR=? (V1,9)		"
		"         THEN				"
		"         ELSE				"
		"         END				"
		"         ENDIF				"
		"         PRINT (\"Parser test for you and for me\",7)	"
		"         LOOP (100)		"
		"         WAIT				";

	std::string *sampleProgramString = new std::string(sampleProgram);
	FCLInstructionVector instructions = getInstructions(sampleProgramString);
	NSLog(@"%lu instructions", instructions->size());*/


	// Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
	// You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
	// If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
	NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
	@throw exception;
	return YES;
}

@end
