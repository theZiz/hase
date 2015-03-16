#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "lobbyList.h"
#include "window.h"
#include "level.h"
#include "about.h"
#include "options.h"

SDL_Surface* screen;
spFontPointer font = NULL;

#define BUTTON_BG spGetRGB(64,64,64)
#define BUTTON_FG spGetRGB(220,220,220)

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
	spFontAdd( font, SP_FONT_GROUP_ASCII"™°∞", BUTTON_FG ); //whole ASCII
	spFontAddBorder(font , 0);
	spFontAddButton( font, 'R', MY_BUTTON_START_NAME, BUTTON_FG, BUTTON_BG ); //Return == START
	spFontAddButton( font, 'B', MY_BUTTON_SELECT_NAME, BUTTON_FG, BUTTON_BG ); //Backspace == SELECT
	spFontAddButton( font, 'l', MY_BUTTON_L_NAME, BUTTON_FG, BUTTON_BG ); // q == L
	spFontAddButton( font, 'r', MY_BUTTON_R_NAME, BUTTON_FG, BUTTON_BG ); // e == R
	spFontAddButton( font, 'o', MY_PRACTICE_OK_NAME, BUTTON_FG, BUTTON_BG ); //a == left button
	spFontAddButton( font, 'c', MY_PRACTICE_CANCEL_NAME, BUTTON_FG, BUTTON_BG ); // d == right button
	spFontAddButton( font, '3', MY_PRACTICE_3_NAME, BUTTON_FG, BUTTON_BG ); // w == up button
	spFontAddButton( font, '4', MY_PRACTICE_4_NAME, BUTTON_FG, BUTTON_BG ); // s == down button
	spFontAddArrowButton( font, '<', SP_BUTTON_ARROW_LEFT, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, '^', SP_BUTTON_ARROW_UP, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, '>', SP_BUTTON_ARROW_RIGHT, BUTTON_FG, BUTTON_BG );
	spFontAddArrowButton( font, 'v', SP_BUTTON_ARROW_DOWN, BUTTON_FG, BUTTON_BG );
	spFontMulWidth(font,spFloatToFixed(0.9f));

	spSetVirtualKeyboard(SP_VIRTUAL_KEYBOARD_IF_NEEDED,0,h-w*48/320,w,w*48/320,spLoadSurface("./data/keyboard320.png"),spLoadSurface("./data/keyboardShift320.png"));
	spSetVirtualKeyboardBackspaceButton(MY_PRACTICE_4);
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
	resize( screen->w, screen->h );
	init_window_sprites();
	int done = 0;
	while (!done)
	{
		spClearTarget(LL_FG);
		pWindow window = create_window(main_menu_feedback,font,"HASE ("VERSION")");
		window->main_menu = 1;
		add_window_element(window,-1,0);
		add_window_element(window,-1,1);
		add_window_element(window,-1,2);
		add_window_element(window,-1,3);
		add_window_element(window,-1,4);
		add_window_element(window,-1,5);
		int res = modal_window(window,resize);
		int sel = window->selection;
		delete_window(window);
		if (res == 1)
			switch (sel)
			{
				case 0:
					window = create_window(local_game_feedback,font,"Create local game");
					add_window_element(window,0,1);
					add_window_element(window,0,2);
					add_window_element(window,0,3);
					res = modal_window(window,resize);
					delete_window(window);
					if (res == 1)
					{
						char buffer[512];
						pGame game = create_game("New game",lo_game_players,lo_game_seconds,create_level_string(buffer,1536,1536,3,3,3),1,lo_game_hares);
						start_lobby_game(font,resize,game);
						delete_game(game);
					}
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
	quit_window_sprites();
	spSoundStopMusic(0);
	spQuitNet();
	spQuitCore();
	spSoundQuit();
	return 0;
}
