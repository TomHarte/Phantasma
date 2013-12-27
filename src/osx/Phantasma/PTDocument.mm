//
//  PTDocument.m
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#import "PTDocument.h"
#include "Game.h"
#include "16bitBinaryLoader.h"
#include "PTOpenGLView.h"
#import <libkern/OSAtomic.h>

@interface PTDocument () <NSWindowDelegate, PTOpenGLViewDelegate>

@property (weak, nonatomic) IBOutlet PTOpenGLView *openGLView;
- (void)displayLinkDidCallback;

@end

typedef enum
{
	PTKeyStateNone	= 0x00,

	PTKeyStateW		= 0x01,
	PTKeyStateS		= 0x02,
	PTKeyStateA		= 0x04,
	PTKeyStateD		= 0x08,

	PTKeyStateUp	= 0x10,
	PTKeyStateDown	= 0x20,
	PTKeyStateLeft	= 0x40,
	PTKeyStateRight = 0x80,
} PTKeyState;

static CVReturn CVDisplayLinkCallback(
	CVDisplayLinkRef displayLink,
	const CVTimeStamp *inNow,
	const CVTimeStamp *inOutputTime, 
	CVOptionFlags flagsIn, 
	CVOptionFlags *flagsOut, 
	void *displayLinkContext)
{
	[(__bridge PTDocument *)displayLinkContext displayLinkDidCallback];
	return kCVReturnSuccess;
}

@implementation PTDocument
{
	Game *_game;
	CVDisplayLinkRef _displayLink;
	volatile int32_t _displayLinkRedrawQueueCount;
	uint8_t _keyState;
}

- (id)init
{
    self = [super init];
    if (self)
	{
		// Add your subclass-specific initialization here.
		CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
		CVDisplayLinkSetOutputCallback(_displayLink, CVDisplayLinkCallback, (__bridge void *)self);
    }
    return self;
}

- (void)dealloc
{
	CVDisplayLinkRelease(_displayLink);
	_displayLink = NULL;
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
	// one suspects there's an easier way to fill a std::vector
	// from an NSData than this, but I'm a C++ novice
	std::vector<uint8_t> dataVector;
	const uint8_t *bytes = (uint8_t *)[data bytes];
	for(NSUInteger index = 0; index < [data length]; index++)
	{
		dataVector.push_back(bytes[index]);
	}

	// we'll want to redraw immediately on window resizes
	self.openGLView.window.delegate = self;

//	CVDisplayLinkStart(_displayLink);

	_game = load16bitBinary(dataVector);

	return !!_game;
}

- (void)setOpenGLView:(PTOpenGLView *)openGLView
{
	_openGLView = openGLView;
	self.openGLView.delegate = self;

	// this will be the only context we use, so we can just make it current
	// here and forget about it
	[self.openGLView.openGLContext makeCurrentContext];
	_game->setupOpenGL();

	[self setupViewport];

	// configure the CV display link
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink,
		(CGLContextObj)[[self.openGLView openGLContext] CGLContextObj],
		(CGLPixelFormatObj)[[self.openGLView pixelFormat] CGLPixelFormatObj]);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	_game->rotateView((float)theEvent.deltaY, (float)theEvent.deltaX, 0.0f);
	[self.openGLView setNeedsDisplay:YES];
}

- (PTKeyState)maskForKeyEvent:(NSEvent *)theEvent
{
	switch(theEvent.keyCode)
	{
		default:	return PTKeyStateNone;
		
		case 13:	return PTKeyStateW;
		case 1:		return PTKeyStateS;
		case 0:		return PTKeyStateA;
		case 2:		return PTKeyStateD;
		
		case 123:	return PTKeyStateLeft;
		case 124:	return PTKeyStateRight;
		case 125:	return PTKeyStateDown;
		case 126:	return PTKeyStateUp;
	}
}

- (void)keyUp:(NSEvent *)theEvent
{
	_keyState &= ~[self maskForKeyEvent:theEvent];
	[self updateKeyState];
}

- (void)keyDown:(NSEvent *)theEvent
{
	_keyState |= [self maskForKeyEvent:theEvent];
	[self updateKeyState];
}

- (void)updateKeyState
{
	if(_keyState)
	{
		if(!CVDisplayLinkIsRunning(_displayLink))
		{
			_game->advanceToTime( (uint32_t)([NSDate timeIntervalSinceReferenceDate] * 1000) );
			CVDisplayLinkStart(_displayLink);
		}
	}
	else
	{
		if(CVDisplayLinkIsRunning(_displayLink))
			CVDisplayLinkStop(_displayLink);
	}

	float sidewaysVelocity = 0.0f;
	float forwardsVelocity = 0.0f;

	if(_keyState&(PTKeyStateW | PTKeyStateUp))		forwardsVelocity = 1.0f;
	if(_keyState&(PTKeyStateS | PTKeyStateDown))	forwardsVelocity = -1.0f;

	if(_keyState&(PTKeyStateA | PTKeyStateLeft))	sidewaysVelocity = 1.0f;
	if(_keyState&(PTKeyStateD | PTKeyStateRight))	sidewaysVelocity = -1.0f;

	_game->setMovementVelocity(sidewaysVelocity, 0.0f, forwardsVelocity);
}

- (void)windowDidResize:(NSNotification *)notification
{
	[self setupViewport];
}

- (void)setupViewport
{
	[self.openGLView.openGLContext makeCurrentContext];

	// setup the projection viewport; allow for possible retina backing
	NSPoint farEdge =
		[self.openGLView convertPointToBacking:NSMakePoint(self.openGLView.bounds.size.width, self.openGLView.bounds.size.height)];
	glViewport(0, 0, (GLsizei)farEdge.x, (GLsizei)farEdge.y);

	// set the aspect ratio we're now using (in OS X pixels seem always to be square;
	// unless you can find an API that says otherwise?)
	_game->setAspectRatio((float)(self.openGLView.bounds.size.width / self.openGLView.bounds.size.height));
}

- (void)displayLinkDidCallback
{
	if(_game && !_displayLinkRedrawQueueCount)
	{
		OSAtomicIncrement32(&_displayLinkRedrawQueueCount);

		// ensure we retain only a weak reference to self, as this
		// instance may be closed and deallocated before the block
		// gets dispatched
		__weak PTDocument *weakSelf = self;
		dispatch_async(dispatch_get_main_queue(),
		^{
			// if strong self is now nil, just return — the direct instance storage
			// access below won't be safe besides anything else
			PTDocument *strongSelf = weakSelf;
			if(!strongSelf) return;

			[strongSelf updateDisplay];

			OSAtomicDecrement32(&strongSelf->_displayLinkRedrawQueueCount);
		});
	}
}

- (void)drawOpenGLView:(PTOpenGLView *)openGLView
{
	[self updateDisplay];
}

- (void)updateDisplay
{
	[self.openGLView.openGLContext makeCurrentContext];

	// ask the game to update to whatever the time is now
	_game->advanceToTime( (uint32_t)([NSDate timeIntervalSinceReferenceDate] * 1000) );

	// draw the current scene
	_game->draw();

	// switch buffers
	glSwapAPPLE();
}

- (void)close
{
//	CVDisplayLinkStop(_displayLink);
	delete _game;
	_game = NULL;

	[super close];
}

@end
