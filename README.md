# Phantasma

A remake of Incentive Software's Freescape engine, allowing games created with the 3d Construction Set to run on modern computers.

Freescape was an early cross-platform scripted 3d game engine, first being used in 1987's Driller for the Spectrum, CPC, C64, Amiga, ST and PC. This project was inspired by 1991's 3D Construction Kit, released as the Virtual Reality Studio in some overseas markets, which allowed users to make their own wholly original games through a combination of modelling and scripting. Such games could be freely redistributed with the Freescape runtime, making Freescape technically the world's first licensed 3d game engine.

WARNING: this code is half a decade old and predates my switch to computer science as a career. It wears its juvenilia visibly. 

I'm hoping to address that.

## Ongoing Work

See the development branch. In that branch I’m cleaning, commenting, refactoring and removing dependencies. It’s work in progress and will return to master when it’s playable.

## Build Requirements

SDL, SDL_mixer, SDL_image, Freetype.

## Outstanding Issues

From memory:
* the collision model is slightly wrong; it takes you to be a hovering cube which attempts to remain a certain distance from the floor (hence the smoothed motion up stairs, etc) but that allows you to do such feats as walking straight over the final barrier in the 8-bit demo game; and
* rendering uses a depth buffer for order determination; the original doesn't so despite some attempted patches via the polygon offset extension (yes, extension — this is olden timesy OpenGL 1.x code) drawing order is not always identical.

## Things I'd Like To Do

1. remove the SDL and Freetype dependencies in favour of direct ties to the host OS; it's very slightly more code to maintain but makes the whole thing so much nicer to use;
2. rewrite the scene drawer not to use a depth buffer, adjust the collision model as per comments above; and
3. probably shift the main part of the code to vanilla C as I was never very good at C++ and am certainly not now.

## Specific Recollections
### FCL Runner
The FCL runner runs Freescape Command Language scripts (called variously 'conditions' or 'animators' in Freescape, depending on what they do). Despite references to compilation in the comments and the method names, it does this by tokenising the scripts, building a (fairly trivial, due to the language — I don't specifically recall whether it is even technically a tree rather than being all but a straight tokenised stream) syntax tree by recursive descent and subsequently by interpreting that. I think this is the correct way to proceed as it keeps the code simple and the processing disparity between machines now and machines then means that interpreting is never going to be a performance problem in practice.

The 8-bit version of the 3d Construction Kit uses a byte-code rather than textual scripts. Its functionality is a subset of the 16-bit language. In this code, 8-bit codes are converted into textual scripts then supplied to the FCL runner.

(note on factoring: the modern me would separate the immutable stuff here from the state stuff, and not make the 'condition' class also responsible for string accumulation)

So, briefly:
* everything up to line 312 is definitely to do with tokenising and building the syntax tree;
* the stuff briefly beyond that (and SetLooping before it) is to do with establishing state for performance;
* Execute walks the syntax tree and interprets it.

### The TXT Loader and ZX Binary Loader
The 16-bit construction set stores projects as plain text (or, possibly, exports to that — I no longer recall). The TXT loader parses that plain text format. I didn't have a formal documentation of the format but it's completely self explanatory. The 3d Construction Kit comes with an example.

The 8-bit construction sets use a binary format. If memory serves it's essentially the same across all three platforms (the Spectrum, the Amstrad and the C64) with the only minor difference being how the palette information is stored. 

I no longer specifically recall where I obtained the documentation on the binary code used on the 8-bit platforms but I explicitly did not reverse engineer it myself.

### Objects
An object is the fundamental unit of the 3d world. In Freescape world every object is either a convex planar polygon, a cuboid or a 'pyramid' (actually a right rectangular frustum). They're all axis aligned. The pyramid may correspondingly have one of six orientations, depending on which direction you want the smaller end to face.

In Freescape, collision detection is specified as occurring against the axis-aligned bounding boxes of these shapes.

(similar factoring note to the above: I think I've put too much into the object class by trying to handle drawing, hit testing and collisions in the same place)

### Areas
Areas are individual scenes — they collect some objects with some scripts (animators and conditions on the 16-bit platforms, conditions only on the 8-bit platforms), set a scale for the scene and have some defined entrances.

User traversal between (and, sometimes, within) areas is handled by adding a condition on the thing that looks like a door that checks for collisions and warps the user to an entrance elsewhere when appropriate.

Note the scene scale. It's sort of like floating point for the entire scene. You trade scene size for geometry precision.

## EBGF
That's event-based game framework. It's a little framework I put together to help with game writing. The fundamental concepts are message passing to communicate events, with all the boring stuff about creating windows, dealing with resizes, etc, all handled for you. It also maintains a stack of screens and has a few maths helpers.

The specific app needs to respond only to the proper messages — update, draw (frame rate compensation is automatic), setup, etc. This project requires very little of EBGF. Check out gamemain.cpp for the interface between Phantasma and EBGF.

It's through the EBGF that the dependencies creep in. As I'd like to eliminate the dependencies in favour of using OS-specific code I'd like to eliminate a large part of this area.