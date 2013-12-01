#ifndef __EBGF_H
#define __EBGF_H

#include "SDL.h"
#include "SDL_opengl.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592654f
#endif

/* the message structure */
enum EBGF_MessageType {EBGF_PROGSTART, EBGF_PROGEND, EBGF_LOADPREFS, EBGF_SAVEPREFS, EBGF_SETUPDISPLAY, EBGF_UPDATE, EBGF_DRAW, EBGF_ENTER, EBGF_EXIT, EBGF_SDLMESSAGE, EBGF_SETDISPLAYPREFERENCES};
struct EBGF_Message
{
	EBGF_MessageType Type;
	Uint32 Age;
	union
	{
		struct{
			SDL_Event *Event;
		} SDLMessage;
		struct{
			FILE *File;
		} Preferences;
	} Data;
};

/* base class for each screen */
class CGameScreen
{
	public:
		virtual void Message(const EBGF_Message &Message) {};
};

/* base class for game */ 
class CGame: public CGameScreen
{
	public:
		virtual CGameScreen *GetFirstScreen() {return this;}
		virtual char *GetShortName() { return NULL; }
};

/* implement this one yourself so that EBGF knows where to start */
extern CGame *GetGame();

/* functions for dictating various things */
extern void EBGF_SetLogicFrequency(int BeatsPerSecond);
extern int EBGF_GetLogicFrequency();
extern void EBGF_PushScreen(CGameScreen *scr);

/* functions for getting the horizontal and vertical field of view - these return a number in degrees! */
extern float EBGF_GetHFOV();
extern float EBGF_GetVFOV();

/* the next two return the current horizontal and vertical screen size in pixels */
extern int EBGF_GetHResolution();
extern int EBGF_GetVResolution();

/* a floating point analogue of rand() - EBGF automatically seeds the random number generator */
extern float flrand();

/* for querying current FPS */
extern float EBGF_GetFPS();

/* functions for getting mouse mickeys */
extern int EBGF_GetMouseMickeys(int &x, int &y);
extern void EBGF_ClearMouseMickeys();
extern void EBGF_GrabMouse(bool allow);

/* a function for manipulating the screen stack */
enum EBGF_StackCommand {EBGF_DESCEND, EBGF_ASCEND, EBGF_REMOVE, EBGF_RESTART};
extern void EBGF_DoStackCommand(EBGF_StackCommand cmd);

/* related functions */
#include "ebgf_Font.h"
#include "ebgf_Texture.h"

#endif
