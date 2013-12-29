//
//  PTOpenGLView.h
//  Phantasma
//
//  Created by Thomas Harte on 26/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PTOpenGLView;
@protocol PTOpenGLViewDelegate <NSObject>

- (void)scrollWheel:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)rotateWithEvent:(NSEvent *)theEvent;

- (void)drawOpenGLView:(PTOpenGLView *)openGLView;

@end

@interface PTOpenGLView : NSOpenGLView

@property (nonatomic, weak) id <PTOpenGLViewDelegate> delegate;

@end
