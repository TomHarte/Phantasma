Phantasma
=========

A remake of Domark's Freescape engine, allowing games created with the 3d Construction Set to run on modern computers.

WARNING: this code is half a decade old and predates my switch to computer science as a career. It wears its juvenilia visibly. 

I'm hoping to address that.

Build Requirements
------------------

SDL, SDL_mixer, SDL_image, Freetype.

Outstanding Issues
------------------

From memory:
* the collision model is slightly wrong; it takes you to be a hovering cube which attempts to remain a certain distance from the floor (hence the smoothed motion up stairs, etc) but that allows you to do such feats as walking straight over the final barrier in the 8-bit demo game; and
* rendering uses a depth buffer for order determination; the original doesn't so despite some attempted patches via the polygon offset extension (yes, extension — this is olden timesy OpenGL 1.x code) drawing order is not always identical.

Things I'd Like To Do
---------------------
1. remove the SDL and Freetype dependencies in favour of direct ties to the host OS; it's very slightly more code to maintain but makes the whole thing so much nicer to use;
2. rewrite the scene drawer not to use a depth buffer, adjust the collision model as per comments above; and
3. probably shift the main part of the code to vanilla C as I was never very good at C++ and am certainly not now.