#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "client.h"
#include "lobbyList.h"
#include "window.h"
#include "level.h"
#include "about.h"
#include "options.h"
#include "mapping.h"

SDL_Surface* screen;
spFontPointer font = NULL;

#define BUTTON_FG spGetRGB(32,32,32)
#define BUTTON_BG spGetRGB(220,220,220)
#define FONT_BG spGetRGB(64,64,64)
#define FONT_FG spGetRGB(220,220,220)

void resize( Uint16 w, Uint16 h )
{
	spSelectRenderTarget(screen);
	spFontSetShadeColor(0);
	//Font Loading
	if ( font )
	{
		if (spGetSizeFactor() > SP_ONE)
			spFontReload( font, "./data/DejaVuSans-Bold.ttf", 8 * spGetSizeFactor() >> SP_ACCURACY);
		else
			spFontReload( font, "./data/PixelManiaConden.ttf", 16 * spGetSizeFactor() >> SP_ACCURACY);
	}
	else
	{
		if (spGetSizeFactor() > SP_ONE)
			font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 8 * spGetSizeFactor() >> SP_ACCURACY);
		else
			font = spFontLoad( "./data/PixelManiaConden.ttf", 16 * spGetSizeFactor() >> SP_ACCURACY);
	}
	spFontAdd( font, SP_FONT_GROUP_ASCII"™°∞", FONT_FG ); //whole ASCII
	spFontAddBorder(font , 0);
	#ifdef DESKTOP
		int i;
		for (i = 0; i < 322; i++)
		{
			char* name = SDL_GetKeyName(i);
			if (name[0] == 0 || strcmp(name,"unknown key") == 0)
			{
				//printf("Überspringe %i: %s\n",i,name);
				continue;
			}
			char buffer[32];
			buffer[0] = toupper(name[0]);
			int j;
			for (j = 1; j < 31 && name[j];j++)
				if (name[j-1] == ' ')
					buffer[j] = toupper(name[j]);
				else
					buffer[j] = name[j];
			buffer[j] = 0;
			//printf("Add %i (%i): %s\n",i,sdlkey_to_char(i),buffer);
			spFontAddButton( font, spMapSDLKeyToChar(i), buffer, BUTTON_FG, BUTTON_BG );
		}
	#else
		spFontAddButton( font, 'R', SP_BUTTON_START_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); //Return == START
		spFontAddButton( font, 'B', SP_BUTTON_SELECT_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); //Backspace == SELECT
		spFontAddButton( font, 'l', SP_BUTTON_L_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); // q == L
		spFontAddButton( font, 'r', SP_BUTTON_R_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); // e == R
		spFontAddButton( font, 'o', SP_PRACTICE_OK_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); //a == left button
		spFontAddButton( font, 'c', SP_PRACTICE_CANCEL_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); // d == right button
		spFontAddButton( font, '3', SP_PRACTICE_3_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); // w == up button
		spFontAddButton( font, '4', SP_PRACTICE_4_NOWASD_NAME, BUTTON_FG, BUTTON_BG ); // s == down button
	#endif
	spFontAddArrowButton( font, '<', SP_BUTTON_ARROW_LEFT, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, '^', SP_BUTTON_ARROW_UP, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, '>', SP_BUTTON_ARROW_RIGHT, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, 'v', SP_BUTTON_ARROW_DOWN, BUTTON_FG, BUTTON_BG );
	spFontMulWidth(font,spFloatToFixed(0.9f));

	spSetVirtualKeyboard(SP_VIRTUAL_KEYBOARD_IF_NEEDED,0,h-w*48/320,w,w*48/320,spLoadSurface("./data/keyboard320.png"),spLoadSurface("./data/keyboardShift320.png"));
	spSetVirtualKeyboardBackspaceButton( spMapPoolByID(MAP_VIEW) );
	if (spGetSizeFactor() <= SP_ONE)
		font->maxheight = font->maxheight-1;
}

int main_menu_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (elem->reference)
	{
		case 0: sprintf(elem->text,"Local Game"); break;
		case 1: sprintf(elem->text,"Online Game"); break;
		case 2: sprintf(elem->text,"Options"); break;
		case 3: sprintf(elem->text,"Help"); break;
		case 4: sprintf(elem->text,"About"); break;
		case 5: sprintf(elem->text,"Exit"); break;
	}
	return 0;
}

int lo_game_players = 4;
int lo_game_seconds = 45;
int lo_game_hares = 3;

int local_game_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_LEFT:
			switch (elem->reference)
			{
				case 1:
					if (lo_game_players > 2)
						lo_game_players--;
					break;
				case 2:
					if (lo_game_seconds > 5)
						lo_game_seconds -= 5;
					break;
				case 3:
					if (lo_game_hares > 1)
						lo_game_hares--;
					break;
			}
			break;
		case WN_ACT_RIGHT:
			switch (elem->reference)
			{
				case 1:
					lo_game_players++;
					break;
				case 2:
					lo_game_seconds += 5;
					break;
				case 3:
					lo_game_hares++;
					break;
			}
			break;
	}
	switch (elem->reference)
	{
		case 1: sprintf(elem->text,"Maximum players: %i",lo_game_players); break;
		case 2: sprintf(elem->text,"Seconds per turn: %i",lo_game_seconds); break;
		case 3: sprintf(elem->text,"Hares per player: %i",lo_game_hares); break;
	}
	return 0;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	spSetRand(time(NULL));
	//spSetDefaultWindowSize( 320, 240 );
	spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	spSetReturnBehavior(1,0);
	spInitNet();
	spSoundInit();
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	load_options();
	save_options();
	spSoundSetMusic("./sounds/Ouroboros.ogg");
	spSoundPlayMusic(0,-1);

	spMapSetMapSet(1);
	#ifdef DESKTOP
		spMapDesktopHack(1);
		spMapPoolAddForDesktopHack();
		spMapButtonAdd(MAP_CHAT,"chat","Chat",SDLK_RETURN);
		spMapButtonAdd(MAP_MENU,"menu","Menu",SDLK_ESCAPE);
		spMapButtonAdd(MAP_JUMP,"jump","Jump",SDLK_LCTRL);
		spMapButtonAdd(MAP_SHOOT,"shoot","Shoot",SDLK_LALT);
		spMapButtonAdd(MAP_WEAPON,"weapon","Weapon",SDLK_MENU);
		spMapButtonAdd(MAP_VIEW,"view","View",SDLK_LSHIFT);
		spMapButtonAdd(MAP_POWER_DN,"power_down","Power down",SDLK_PAGEDOWN);
		spMapButtonAdd(MAP_POWER_UP,"power_up","Power up",SDLK_PAGEUP);
	#else
		spMapPoolAdd(SP_BUTTON_START_NOWASD,"[R]");
		spMapPoolAdd(SP_BUTTON_SELECT_NOWASD,"[B]");
		spMapPoolAdd(SP_BUTTON_L_NOWASD,"[l]");
		spMapPoolAdd(SP_BUTTON_R_NOWASD,"[r]");
		spMapPoolAdd(SP_PRACTICE_OK_NOWASD,"[o]");
		spMapPoolAdd(SP_PRACTICE_CANCEL_NOWASD,"[c]");
		spMapPoolAdd(SP_PRACTICE_3_NOWASD,"[3]");
		spMapPoolAdd(SP_PRACTICE_4_NOWASD,"[4]");
		spMapButtonAdd(MAP_CHAT,"chat","Chat",SP_BUTTON_START_NOWASD);
		spMapButtonAdd(MAP_MENU,"menu","Menu",SP_BUTTON_SELECT_NOWASD);
		spMapButtonAdd(MAP_JUMP,"jump","Jump",SP_PRACTICE_OK_NOWASD);
		spMapButtonAdd(MAP_SHOOT,"shoot","Shoot",SP_PRACTICE_CANCEL_NOWASD);
		spMapButtonAdd(MAP_WEAPON,"weapon","Weapon",SP_PRACTICE_3_NOWASD);
		spMapButtonAdd(MAP_VIEW,"view","View",SP_PRACTICE_4_NOWASD);
		spMapButtonAdd(MAP_POWER_DN,"power_down","Power down",SP_BUTTON_L_NOWASD);
		spMapButtonAdd(MAP_POWER_UP,"power_up","Power up",SP_BUTTON_R_NOWASD);
	#endif
	spMapSetStrategy(SP_MAPPING_SWITCH);
	spMapLoad("hase","controls.cfg");
	spMapSave("hase","controls.cfg");
	spMapSetMapSet(0);
	#ifdef DESKTOP
		spMapPoolAddForDesktopHack();
		spMapButtonAdd(MAP_CHAT,"chat","Chat",SDLK_RETURN);
		spMapButtonAdd(MAP_MENU,"menu","Back",SDLK_ESCAPE);
		spMapButtonAdd(MAP_JUMP,"jump","Okay",SDLK_LCTRL);
		spMapButtonAdd(MAP_SHOOT,"shoot","Cancel",SDLK_LALT);
		spMapButtonAdd(MAP_WEAPON,"weapon","New game / Insult",SDLK_MENU);
		spMapButtonAdd(MAP_VIEW,"view","Spectate",SDLK_LSHIFT);
		spMapButtonAdd(MAP_POWER_DN,"power_down","Select left",SDLK_PAGEDOWN);
		spMapButtonAdd(MAP_POWER_UP,"power_up","Select right",SDLK_PAGEUP);
	#else
		spMapPoolAdd(SP_BUTTON_START_NOWASD,"[R]");
		spMapPoolAdd(SP_BUTTON_SELECT_NOWASD,"[B]");
		spMapPoolAdd(SP_BUTTON_L_NOWASD,"[l]");
		spMapPoolAdd(SP_BUTTON_R_NOWASD,"[r]");
		spMapPoolAdd(SP_PRACTICE_OK_NOWASD,"[o]");
		spMapPoolAdd(SP_PRACTICE_CANCEL_NOWASD,"[c]");
		spMapPoolAdd(SP_PRACTICE_3_NOWASD,"[3]");
		spMapPoolAdd(SP_PRACTICE_4_NOWASD,"[4]");
		spMapButtonAdd(MAP_CHAT,"chat","Chat",SP_BUTTON_START_NOWASD);
		spMapButtonAdd(MAP_MENU,"menu","Back",SP_BUTTON_SELECT_NOWASD);
		spMapButtonAdd(MAP_JUMP,"jump","Okay",SP_PRACTICE_OK_NOWASD);
		spMapButtonAdd(MAP_SHOOT,"shoot","Cancel",SP_PRACTICE_CANCEL_NOWASD);
		spMapButtonAdd(MAP_WEAPON,"weapon","New game / Insult",SP_PRACTICE_3_NOWASD);
		spMapButtonAdd(MAP_VIEW,"view","Spectate",SP_PRACTICE_4_NOWASD);
		spMapButtonAdd(MAP_POWER_DN,"power_down","Select left",SP_BUTTON_L_NOWASD);
		spMapButtonAdd(MAP_POWER_UP,"power_up","Select right",SP_BUTTON_R_NOWASD);
	#endif
	spMapSetStrategy(SP_MAPPING_SWITCH);
	spMapLoad("hase","menu.cfg");
	spMapSave("hase","menu.cfg");

	resize( screen->w, screen->h );
	init_window_sprites();

	int done = 0;
	pWindow window = create_window(main_menu_feedback,font,"HASE ("VERSION")");
	window->main_menu = 1;
	add_window_element(window,-1,0);
	add_window_element(window,-1,1);
	add_window_element(window,-1,2);
	add_window_element(window,-1,3);
	add_window_element(window,-1,4);
	add_window_element(window,-1,5);
	pWindow subWindow;
	while (!done)
	{
		spClearTarget(LL_FG);
		if (modal_window(window,resize) == 1)
			switch (window->selection)
			{
				case 0:
					subWindow = create_window(local_game_feedback,font,"Create local game");
					add_window_element(subWindow,0,1);
					add_window_element(subWindow,0,2);
					add_window_element(subWindow,0,3);
					if (modal_window(subWindow,resize) == 1)
					{
						char buffer[512];
						pGame game = create_game("New game",lo_game_players,lo_game_seconds,create_level_string(buffer,1536,1536,3,3,3),1,lo_game_hares);
						start_lobby_game(font,resize,game,0);
						delete_game(game);
					}
					delete_window(subWindow);
					break;
				case 1:
					if (text_box(font,resize,"Enter player name:",gop_username(),32,0,NULL,0) == 1)
						start_lobby(font,resize);
					break;
				case 2:
					options_window(font,resize,0);
					break;
				case 3:
					start_help(font,resize);
					break;
				case 4:
					start_about(font,resize);
					break;
				case 5:
					done = 1;
					break;
			}
		else
			done = 1;
	}	
	delete_window(window);
	quit_window_sprites();
	spSoundStopMusic(0);
	spQuitNet();
	spQuitCore();
	spSoundQuit();
	return 0;
}
