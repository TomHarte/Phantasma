#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "ebgf.h"
#include "ebgf_Platform.h"
#include "ebgf_ResourceStore.h"
#include "ebgf_GLExts.h"

static SDL_Surface *__EBGF_screen;
int __EBGF_MickeyX, __EBGF_MickeyY, __EBGF_MouseButtons;
int __EBGF_GrabMouseMask;
int EBGF_GetMouseMickeys(int &X, int &Y)
{
	X = __EBGF_MickeyX; __EBGF_MickeyX = 0;
	Y = __EBGF_MickeyY; __EBGF_MickeyY = 0;

	int OldMask = __EBGF_GrabMouseMask;
	__EBGF_GrabMouseMask &= __EBGF_MouseButtons;
	return __EBGF_MouseButtons &~OldMask;
}

void EBGF_ClearMouseMickeys()
{
	__EBGF_MickeyX = __EBGF_MickeyY = 0;
}

float __EBGF_HFOV, __EBGF_VFOV;
int __EBGF_HRES, __EBGF_VRES;

float EBGF_GetHFOV()
{
	return __EBGF_HFOV;
}

float EBGF_GetVFOV()
{
	return __EBGF_VFOV;
}

int EBGF_GetHResolution()
{
	return __EBGF_HRES;
}

int EBGF_GetVResolution()
{
	return __EBGF_VRES;
}

bool __EBGF_AllowGrabMouse = false;

void EBGF_GrabMouse(bool allow)
{
	__EBGF_AllowGrabMouse = allow;

	/* to make sure the mouse is released, post a NOP event */
	SDL_Event NewEvent;
	NewEvent.type = SDL_USEREVENT;
	SDL_PushEvent(&NewEvent);
}

float flrand()
{
	return (float)rand() / RAND_MAX;
}

bool __EBGF_SetupGFX(int w, int h, bool fs, bool ms, bool hr, float Gamma)
{
	/* if fs, find new w, h */
	if(fs)
	{
		SDL_Rect **Modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);
		SDL_Rect *Res;
		if(hr)
			Res = Modes[0];
		else
		{
			/* find first mode with height = 480, or take first higher */
			int index = 0;
			while(Modes[index] && Modes[index]->h > 480)
				index++;
			if(!Modes[index] || Modes[index]->h < 480) index--;
			Res = Modes[index];
		}

		w = Res->w;
		h = Res->h;
	}

	/* store pixel resolution (multisampling allowed for) */
	__EBGF_HRES = h;
	__EBGF_VRES = w;

//	if(ms)
//	{
//		__EBGF_HRES <<= 2;
//		__EBGF_VRES <<= 2;
//	}

	/* store fields of view */
	__EBGF_VFOV = 45.0f;
	__EBGF_HFOV = ((float)w / (float)h) * 45.0f;

	/* don't accept HFOV > 180 */
	if(__EBGF_HFOV > 180)
	{
		h = (int)((float)w / (180.0f / 45.0f));
		__EBGF_HFOV = ((float)w / (float)h) * 45.0f;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, ms ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, ms ? 4 : 0);

	if(__EBGF_screen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (fs ? SDL_FULLSCREEN : SDL_RESIZABLE)))
	{	
		/* default projection is a 45 degree vertical FOV, positive z = further into screen, perspective, z range 1 -> 1000 */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, (float)w / (float)h, 0.1, 10000);
//		glScalef(1.0f, 1.0f, -1.0f);

		/* default modelview is identity */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		/* enable depth test */
		glEnable(GL_DEPTH_TEST);

		/* enable reverse face removal */
		glEnable(GL_CULL_FACE);

		/* set "ordinary" alpha */
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		/* set gamma */
//		SDL_SetGamma(Gamma, Gamma, Gamma);
		__ebgf_GetExtensions();
	}

	return __EBGF_screen != NULL ? true : false;
}

CGameScreen *__EBGF_GameScreenStack[32];
int __EBGF_GameStackReadPointer, __EBGF_GameStackWritePointer;

void EBGF_PushScreen(CGameScreen *scr)
{
	__EBGF_GameScreenStack[__EBGF_GameStackWritePointer] = scr;
	__EBGF_GameStackWritePointer++;
}

void EBGF_SyncWritePointer()
{
	if(__EBGF_GameStackWritePointer <= __EBGF_GameStackReadPointer) return;
	while(1)
	{
		__EBGF_GameStackWritePointer--;
		if(__EBGF_GameStackWritePointer > __EBGF_GameStackReadPointer)
			delete __EBGF_GameScreenStack[__EBGF_GameStackWritePointer];
		else
		{
			__EBGF_GameStackWritePointer++;
			return;
		}
	}
}

float __EBGF_FPS = 0;
float EBGF_GetFPS()
{
	return __EBGF_FPS;
}

EBGF_StackCommand __EBGF_PendingCommands[32];
int __EBGF_CommandPointer;
void EBGF_DoStackCommand(EBGF_StackCommand cmd)
{
	__EBGF_PendingCommands[__EBGF_CommandPointer] = cmd;
	__EBGF_CommandPointer++;
}

int __EBGF_BeatsPerSecond;
void EBGF_SetLogicFrequency(int bps)
{
	__EBGF_BeatsPerSecond = bps;
}

int EBGF_GetLogicFrequency()
{
	return __EBGF_BeatsPerSecond;
}

#define SendDoubleMessage()	\
	__EBGF_AttachedGame->Message(msg);\
	if(__EBGF_GameScreenStack[__EBGF_GameStackReadPointer] != __EBGF_AttachedGame) __EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);

CGame *__EBGF_AttachedGame;

int main(int argc, char *argv[])
{
	// initialise SDL, seed random number generator
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		exit(1);
	srand(time(NULL));

	// request GL double buffer, 32bit depth buffer, 8bit z-buffer, vsync on, accelerated graphics if possible
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// set defaults: high res when full screen, no multisampling, mouse not current grabbed
	bool HighRes = false;
	bool MultiSample = true;
	bool GrabMouse = false;

	// other defaults: 640x480 windowed mode, with gamma 1
	int Width = 640, Height = 480;
	bool FullScr = false;
	float Gamma = 1.0f;

	// default logic rate: 100 bps
	__EBGF_BeatsPerSecond = 100;

	// open audio at 44100 Hz in the default format with a large 4096 sample buffer
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 4096);

	// seed resource list
	__EBGF_CreateResourceList();

	// get CGame derivative
	__EBGF_AttachedGame = GetGame();
	EBGF_Message msg;

	// give a pre-GFX mode chance to stipulate z, stencil, etc 
	msg.Age = 0;
	msg.Type = EBGF_SETDISPLAYPREFERENCES;
	__EBGF_AttachedGame->Message(msg);

	// get first screen, push it to stack, initialise screen stack as empty
	__EBGF_GameStackReadPointer = __EBGF_GameStackWritePointer = 0;
	__EBGF_CommandPointer = 0;
	EBGF_PushScreen(__EBGF_AttachedGame->GetFirstScreen());

	// figure out filename for preferences
	char SaveName[2048], *ShortName = __EBGF_AttachedGame->GetShortName();
	if(!ShortName) ShortName = "ebgfprefs";
	sprintf(SaveName, "%s%s.cfg", __EBGF_GetSavePath(), ShortName);

	// load preferences if possible
	FILE *SaveHandle;
	if(SaveHandle = fopen(SaveName, "rb"))
	{
		// get highres, multisample & fullscr settings
		Uint8 Flags = fgetc(SaveHandle);
		HighRes = (Flags&1) ? true : false;
		MultiSample = (Flags&2) ? true : false;
		GrabMouse = FullScr = (Flags&4) ? true : false;
		SDL_ShowCursor(GrabMouse ? SDL_DISABLE : SDL_ENABLE);

		// get window size
		Width = fgetc(SaveHandle);
		Width |= fgetc(SaveHandle) << 8;
		Width |= fgetc(SaveHandle) << 16;
		Width |= fgetc(SaveHandle) << 24;
		Height = fgetc(SaveHandle);
		Height |= fgetc(SaveHandle) << 8;
		Height |= fgetc(SaveHandle) << 16;
		Height |= fgetc(SaveHandle) << 24;

		// get gamma
		Uint32 FixGamma;
		FixGamma = fgetc(SaveHandle);
		FixGamma |= fgetc(SaveHandle) << 8;
		FixGamma |= fgetc(SaveHandle) << 16;
		FixGamma |= fgetc(SaveHandle) << 24;
		Gamma = (float)FixGamma / 65536.0f;

		// let the attached game load whatever it wants - highscores, options, etc
		msg.Type = EBGF_LOADPREFS;
		msg.Data.Preferences.File = SaveHandle;
		__EBGF_AttachedGame->Message(msg);

		fclose(SaveHandle);
	}

	// create display
	if(!__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma))
	{
		printf("Unable to open graphics mode!\n");
		exit(1);
	}

	// pass on PROGSTART
	msg.Age = 0;
	msg.Type = EBGF_PROGSTART;
	__EBGF_AttachedGame->Message(msg);

	// allow game and scren to setup special graphics requirements
	msg.Type = EBGF_SETUPDISPLAY;
	SendDoubleMessage();

	// send initial ENTER message
	msg.Type = EBGF_ENTER;
	__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);

	// seed event loop
	bool Quit = false;
	Uint32 OldTime = SDL_GetTicks(), SpareMS = 0;

	int FrameCount = 0;
	Uint32 FrameCalcTime = SDL_GetTicks();
	
	// main game loop: run while not requested to quit
	while(!Quit)
	{
		// process latest SDL events
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			bool FixRes = false, OldGrabMouse = GrabMouse;
			switch(event.type)
			{
				default: break;
				case SDL_QUIT:
					Quit = true;
				break;
				case SDL_VIDEORESIZE:
					Width = event.resize.w;
					Height = event.resize.h;
					FixRes = true;
				break;
				case SDL_MOUSEBUTTONDOWN:
					GrabMouse = true;
					if(!OldGrabMouse) __EBGF_GrabMouseMask = SDL_GetMouseState(NULL, NULL);
				break;
				case SDL_ACTIVEEVENT:
					if(!event.active.gain && event.active.state && !FullScr)
						GrabMouse = false;
				break;
				case SDL_KEYDOWN:
					/* some special key downs: ESC returns to the top screen or quits if the program is already there */
					switch(event.key.keysym.sym)
					{
						default: break;
						case SDLK_ESCAPE:
							if(!__EBGF_GameStackReadPointer) Quit = true;
							else
							{
								msg.Type = EBGF_EXIT;
								__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);

								__EBGF_GameStackReadPointer = 0; msg.Age = 0;
								EBGF_SyncWritePointer();

								msg.Type = EBGF_ENTER;
								__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);
								msg.Type = EBGF_SETUPDISPLAY;
								__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);
							}
						break;
						case SDLK_b:
							Gamma += 0.15f;
							if(Gamma >= 2.5f)
								Gamma = 1.0f;
							SDL_SetGamma(Gamma, Gamma, Gamma);
						break;
 						case SDLK_m:
							MultiSample ^= true;
							FixRes = true;
						break;
 						case SDLK_h:
							if(FullScr)
							{
								HighRes ^= true;
								FixRes = true;
							}
						break;
						case SDLK_f:
							if(event.key.keysym.mod&(KMOD_LMETA|KMOD_RMETA))
							{
								FullScr ^= true;
								FixRes = true;
							}
						break;
						case SDLK_RETURN:
							if(event.key.keysym.mod&(KMOD_LALT|KMOD_RALT))
							{
								FullScr ^= true;
								FixRes = true;
							}
						break;
						case SDLK_LALT:
						case SDLK_RALT:
 							if(event.key.keysym.mod&(KMOD_LCTRL|KMOD_RCTRL) && !FullScr)
								GrabMouse = false;
						break;
						case SDLK_LCTRL:
						case SDLK_RCTRL:
 							if(event.key.keysym.mod&(KMOD_LALT|KMOD_RALT) && !FullScr)
								GrabMouse = false;
						break;
					}

				break;
			}
			GrabMouse &= __EBGF_AllowGrabMouse;

			/* check if we need to fix the video resolution */
			if(FixRes)
			{
				__EBGF_BackupResources();

				if(FullScr)
				{
					GrabMouse = true;
					if(!__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma))
					{
						FullScr = false;
						if(!__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma))
						{
							MultiSample = false;
							__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma);
						}
					}
				}
				else
				{
					if(!__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma))
					{
						MultiSample = false;
						__EBGF_SetupGFX(Width, Height, FullScr, MultiSample, HighRes, Gamma);
					}
				}

				/* send setup display messages */
				msg.Type = EBGF_SETUPDISPLAY;
				SendDoubleMessage();

				/* fix grab mouse maybe */
				if(GrabMouse)
					SDL_WarpMouse(__EBGF_screen->w >> 1, __EBGF_screen->h >> 1);

				__EBGF_RestoreResources();
			}

			if(OldGrabMouse != GrabMouse && !FullScr)
				SDL_ShowCursor(GrabMouse ? SDL_DISABLE : SDL_ENABLE);
			else
				SDL_ShowCursor((GrabMouse | FullScr) ? SDL_DISABLE : SDL_ENABLE);

			/* pass it on too! */
			msg.Type = EBGF_SDLMESSAGE;
			msg.Data.SDLMessage.Event = &event;
			__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);
		}

		// check mouse stuff
		if(GrabMouse)
		{
			int NewX, NewY;
			__EBGF_MouseButtons = SDL_GetMouseState(&NewX, &NewY);
			SDL_WarpMouse(__EBGF_screen->w >> 1, __EBGF_screen->h >> 1);
			__EBGF_MickeyX += NewX - (__EBGF_screen->w >> 1);
			__EBGF_MickeyY += NewY - (__EBGF_screen->h >> 1);
		}

		// request redraw
		msg.Type = EBGF_DRAW;
		SendDoubleMessage();

		// flip buffers — execution will pause here if we're ahead of the display frame rate
		SDL_GL_SwapBuffers();
		FrameCount++;

		// consider updating FPS measure
		Uint32 FTime = SDL_GetTicks();
		FTime -= FrameCalcTime;
		if(FTime > 500)
		{
			__EBGF_FPS = (float)(FrameCount * 1000) / FTime;
			FrameCount = 0;
			FrameCalcTime += FTime;
		}

		// work out how many logic updates that makes
		Uint32 ThousandTicks, Ticks, ElapsedTime;
		while(1)
		{
			ElapsedTime = SDL_GetTicks();
			ElapsedTime -= OldTime;

			ThousandTicks = (ElapsedTime * __EBGF_BeatsPerSecond) + SpareMS;
			Ticks = ThousandTicks / 1000;

			if(!Ticks)
				SDL_Delay( ((1000 - ThousandTicks) + (__EBGF_BeatsPerSecond >> 1)) / __EBGF_BeatsPerSecond);
			else
				break;
		}
		OldTime += ElapsedTime;

		// progress by at most ElapsedTime ms
		SpareMS = ThousandTicks % 1000;
		while(Ticks-- && !__EBGF_CommandPointer)
		{
			msg.Type = EBGF_UPDATE;
			msg.Age++;
			SendDoubleMessage();
		}

		// process stack commands
		if(__EBGF_CommandPointer)
		{
			msg.Age = 0;
			msg.Type = EBGF_EXIT;
			__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);

			while(__EBGF_CommandPointer)
			{
				__EBGF_CommandPointer--;
				switch(__EBGF_PendingCommands[__EBGF_CommandPointer])
				{
					case EBGF_DESCEND:
						/* descend means "go one down the stack" */
						__EBGF_GameStackReadPointer++;
					break;
					case EBGF_ASCEND:
						/* ascend means "go one up the stack, forget everything after me" */
						__EBGF_GameStackReadPointer--;
						EBGF_SyncWritePointer();
					break;
					case EBGF_REMOVE:
					{
						/* remove means "cut me out, shuffle everything after me up a spot" */
						delete __EBGF_GameScreenStack[__EBGF_GameStackReadPointer];
						int c = __EBGF_GameStackReadPointer;
						while(c < __EBGF_GameStackWritePointer)
						{
							__EBGF_GameScreenStack[c] = __EBGF_GameScreenStack[c+1];
							c++;
						}					
					}
					break;
					case EBGF_RESTART:
						/* restart means "don't do anything to the stack, but send me a new exit/enter" */
					break;
				}
			}

			msg.Type = EBGF_ENTER;
			__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);
			msg.Type = EBGF_SETUPDISPLAY;
			__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);
		}
	}

	/* exiting current screen... */
	msg.Type = EBGF_EXIT;
	__EBGF_GameScreenStack[__EBGF_GameStackReadPointer]->Message(msg);

	/* end of game, shut everything down */
	__EBGF_GameStackReadPointer = 0;
	EBGF_SyncWritePointer();

	/* save whatever settings we can */
	if(SaveHandle = fopen(SaveName, "wb"))
	{
		/* get highres, multisample & fullscr settings */
		Uint8 Flags = (HighRes ? 1 : 0) | (MultiSample ? 2 : 0) | (FullScr ? 4 : 0);
		fputc(Flags, SaveHandle);

		/* get window size */
		fputc(Width&0xff, SaveHandle);
		fputc((Width >> 8)&0xff, SaveHandle);
		fputc((Width >> 16)&0xff, SaveHandle);
		fputc((Width >> 24)&0xff, SaveHandle);
		fputc(Height&0xff, SaveHandle);
		fputc((Height >> 8)&0xff, SaveHandle);
		fputc((Height >> 16)&0xff, SaveHandle);
		fputc((Height >> 24)&0xff, SaveHandle);

		/* get gamma */
		Uint32 FixGamma = (int)(Gamma * 65536.0f);
		fputc(FixGamma&0xff, SaveHandle);
		fputc((FixGamma >> 8)&0xff, SaveHandle);
		fputc((FixGamma >> 16)&0xff, SaveHandle);
		fputc((FixGamma >> 24)&0xff, SaveHandle);

		/* let the attached game save what it wants */
		msg.Type = EBGF_SAVEPREFS;
		msg.Data.Preferences.File = SaveHandle;
		__EBGF_AttachedGame->Message(msg);

		fclose(SaveHandle);
	}

	msg.Type = EBGF_PROGEND;
	__EBGF_AttachedGame->Message(msg);

	delete __EBGF_AttachedGame;

	// Cleanup
	__EBGF_DestroyResourceList();
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}
