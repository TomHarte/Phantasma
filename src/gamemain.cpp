#include "ebgf/ebgf.h"
#include "Freescape.h"
#include "FreescapeDemoHandler.h"
#include <math.h>

class CMyGame: public CGame
{
	public:
		void Message(const EBGF_Message &Message);
		char *GetShortName() { return "Phantasma"; }
//		CGameScreen *GetFirstScreen() {return new CTitleScreen;}
};

CFreescapeGame *Game = NULL;
CFSDemoHandler *Handler = NULL;

void CMyGame::Message(const EBGF_Message &Message)
{
	switch(Message.Type)
	{
		default: break;
		case EBGF_SETUPDISPLAY:
			SDL_WM_SetCaption("Phantasma", "Phantasma");
			glEnable(GL_CULL_FACE);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_POLYGON_OFFSET_LINE);
			if(Game) Game->SetupDisplay(); // for clear colour mostly
			EBGF_SetLogicFrequency(100);
		break;
		case EBGF_SETDISPLAYPREFERENCES:
			// don't actually want stencil or depth buffers...
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
		break;
		case EBGF_ENTER:
		{
			SDL_EnableUNICODE(SDL_ENABLE);

			Game = new CFreescapeGame;
			Game->SetFont("Tempest.ttf");

			Game->SetPalette("VGAGAME.PAL");
			Game->LoadSounds("sound");
			Game->OpenTXT("GAME.TXT");
			Handler = new CFSDemoHandler();
			Game->SetHandler(Handler);
			Game->Reset();
			Game->SetupDisplay();

			EBGF_GrabMouse(true);
			EBGF_ClearMouseMickeys();
		}
		break;
		case EBGF_EXIT:
			delete Game; Game = NULL;
			delete Handler; Handler = NULL;
		break;
		case EBGF_SDLMESSAGE:
			if(Game && Message.Data.SDLMessage.Event->type == SDL_KEYDOWN)
			{
				if(Message.Data.SDLMessage.Event->key.keysym.unicode < 128)
					Game->PostASCII(Message.Data.SDLMessage.Event->key.keysym.unicode);
			}
		break;
		case EBGF_UPDATE:
		{
			static int OldButtons = 0;
			static Uint8 OldSpace = 0;
			int Buttons = 0;
			Uint8 *Keys = SDL_GetKeyState(NULL);

			if(Keys)
			{
				int MoveVec[6] = {0, 0, 0, 0, 0, 0};
				if(Keys[SDLK_UP] || Keys[SDLK_w]) MoveVec[2] = 5;
				if(Keys[SDLK_DOWN] || Keys[SDLK_s]) MoveVec[2] = -5;
				if(Keys[SDLK_LEFT] || Keys[SDLK_a]) MoveVec[0] = -5;
				if(Keys[SDLK_RIGHT] || Keys[SDLK_d]) MoveVec[0] = 5;
				if(Keys[SDLK_q]) MoveVec[5] = -3;
				if(Keys[SDLK_e]) MoveVec[5] = 3;
				Buttons = 4;
				Buttons = EBGF_GetMouseMickeys(MoveVec[4], MoveVec[3]);
				Game->Move(MoveVec[0], MoveVec[1], MoveVec[2], MoveVec[3], MoveVec[4], MoveVec[5]);

				if((Buttons^OldButtons)&Buttons&1) Game->Shoot();
				if((Buttons^OldButtons)&Buttons&4) Game->Activate();
				if((OldSpace^Keys[SDLK_SPACE])&Keys[SDLK_SPACE]) Game->ToggleCrouch();
				OldSpace = Keys[SDLK_SPACE];
			}

			Game->Update();

			OldButtons = Buttons;
		}
		break;
		case EBGF_DRAW:
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Game->Draw();
		break;
	}
}

CGame *GetGame()
{
	return new CMyGame;
}
