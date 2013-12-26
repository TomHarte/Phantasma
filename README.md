# Phantasma

A remake of Incentive Software's Freescape engine, allowing games created with the 3d Construction Set to run on modern computers.

Freescape was an early cross-platform scripted 3d game engine, first being used in 1987's Driller for the Spectrum, CPC, C64, Amiga, ST and PC. This project was inspired by 1991's 3D Construction Kit, released as the Virtual Reality Studio in some overseas markets, which allowed users to make their own wholly original games through a combination of modelling and scripting. Such games could be freely redistributed with the Freescape runtime, making Freescape technically the world's first licensed 3d game engine.

## Ongoing Work

This is the development branch; I’m cleaning, commenting, refactoring and removing dependencies. It’s work in progress and will return to master when it’s playable.

Substitute "will be" for "is" (or as required for tense) where appropriate below.

## Design and Dependencies

An instance of `Game` can be obtained from the binary image of either a 16- or 8-bit 3d Construction Kit game. That instance can then be instructed in user input and requested to issue drawing commands for the 3d display. By supplying a delegate, it can make let you know when a sound effect is triggered or a HUD item needs to be updated.

So the only requirements for the main body of code are a C++ compiler with some form of OpenGL available.

The OpenGL code is designed for the programmable pipeline rather than the fixed and should work across both vanilla OpenGL and OpenGL ES. It gets no benefit from the programmable pipeline but at the time of writing the fixed is a bit of an anachronism and it seems likely it will cease to be available on certain mobile platforms in the near future.

## Modules
### Command Language
Freescape is scripted via the Freescape Command Language ('FCL'); scripts are called either 'conditions' or 'animators', depending on what they do.

In Phantasma, scripts are detokenised from disk, then parsed into an internal form. That's because FCL exists in at least three forms with distinct on-disk tokenisations, so it's helpful to decouple the logic for reading from disk and the logic for performing scripts.

### Areas
Areas are individual scenes — they collect some objects with some scripts (animators and conditions on the 16-bit platforms, conditions only on the 8-bit platforms), set a scale for the scene and have some defined entrances.

User traversal between (and, sometimes, within) areas is handled by adding a condition on the thing that looks like a door that checks for collisions and warps the user to an entrance elsewhere when appropriate.

Note the scene scale. It's sort of like floating point for the entire scene. You trade scene size for geometry precision.