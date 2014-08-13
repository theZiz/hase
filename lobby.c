#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "lobbyList.h"
#include "window.h"
#include "level.h"
#include "about.h"

SDL_Surface* screen;
spFontPointer font;

#define BUTTON_BG spGetRGB(64,64,64)
#define BUTTON_FG spGetRGB(220,220,220)

void resize( Uint16 w, Uint16 h )
{
	spSelectRenderTarget(screen);
	//Font Loading
	if ( font )
		spFontDelete( font );
	spFontSetShadeColor(0);
	if (spGetSizeFactor() > SP_ONE)
		font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 8 * spGetSizeFactor() >> SP_ACCURACY);
	else
		font = spFontLoad( "./data/PixelManiaConden.ttf", 16 * spGetSizeFactor() >> SP_ACCURACY);
	spFontAdd( font, SP_FONT_GROUP_ASCII"™", BUTTON_FG ); //whole ASCII
	spFontAddBorder(font , 0);
	spFontAddButton( font, 'R', MY_BUTTON_START_NAME, BUTTON_FG, BUTTON_BG ); //Return == START
	spFontAddButton( font, 'B', MY_BUTTON_SELECT_NAME, BUTTON_FG, BUTTON_BG ); //Backspace == SELECT
	spFontAddButton( font, 'l', MY_BUTTON_L_NAME, BUTTON_FG, BUTTON_BG ); // q == L
	spFontAddButton( font, 'r', MY_BUTTON_R_NAME, BUTTON_FG, BUTTON_BG ); // e == R
	spFontAddButton( font, 'o', MY_PRACTICE_OK_NAME, BUTTON_FG, BUTTON_BG ); //a == left button
	spFontAddButton( font, 'c', MY_PRACTICE_CANCEL_NAME, BUTTON_FG, BUTTON_BG ); // d == right button
	spFontAddButton( font, '3', MY_PRACTICE_3_NAME, BUTTON_FG, BUTTON_BG ); // w == up button
	spFontAddButton( font, '4', MY_PRACTICE_4_NAME, BUTTON_FG, BUTTON_BG ); // s == down button
	spFontMulWidth(font,spFloatToFixed(0.9f));

	spSetVirtualKeyboard(SP_VIRTUAL_KEYBOARD_IF_NEEDED,0,h-w*48/320,w,w*48/320,spLoadSurface("./data/keyboard320.png"),spLoadSurface("./data/keyboardShift320.png"));
	//if (spGetSizeFactor() <= SP_ONE)
	//	font->maxheight = font->maxheight*4/5;
}

int main_menu_feedback( pWindowElement elem, int action )
{
	switch (elem->reference)
	{
		case 0: sprintf(elem->text,"Local Game"); break;
		case 1: sprintf(elem->text,"Online Game"); break;
		case 2: sprintf(elem->text,"Help"); break;
		case 3: sprintf(elem->text,"About"); break;
		case 4: sprintf(elem->text,"Exit"); break;
	}
	return 0;
}

int lo_game_players = 8;
int lo_game_seconds = 30;
int lo_game_hares = 5;

int local_game_feedback( pWindowElement elem, int action )
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
	spSetRand(time(NULL));
	//spSetDefaultWindowSize( 320, 240 );
	spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	spInitNet();
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	resize( screen->w, screen->h );
	int done = 0;
	while (!done)
	{
		spClearTarget(LL_BG);
		pWindow window = create_window(main_menu_feedback,font,"HASE ("VERSION")");
		window->main_menu = 1;
		add_window_element(window,-1,0);
		add_window_element(window,-1,1);
		add_window_element(window,-1,2);
		add_window_element(window,-1,3);
		add_window_element(window,-1,4);
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
					start_lobby(font,resize);
					break;
				case 2:
					start_help(font,resize);
					break;
				case 3:
					start_about(font,resize);
					break;
				case 4:
					done = 1;
					break;
			}
		else
			done = 1;
	}
	spQuitNet();
	spQuitCore();
	return 0;
}
