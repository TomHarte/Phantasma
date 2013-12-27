//
//  PTOpenGLView.m
//  Phantasma
//
//  Created by Thomas Harte on 26/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#import "PTOpenGLView.h"

@implementation PTOpenGLView

- (void)scrollWheel:(NSEvent *)theEvent	{	[self.delegate scrollWheel:theEvent];	}
- (void)keyDown:(NSEvent *)theEvent		{	[self.delegate keyDown:theEvent];		}
- (void)keyUp:(NSEvent *)theEvent		{	[self.delegate keyUp:theEvent];			}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[self.delegate drawOpenGLView:self];
	[super drawRect:dirtyRect];
}

@end
