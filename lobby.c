#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

SDL_Surface* screen;
spFontPointer font;
int global_counter;
pGame gameList;
int gameCount;

void draw(void)
{
	spClearTarget(0);
	spFontDrawMiddle( screen->w/2, 2+0*font->maxheight/2, 0, "Hase Lobby", font );
	char buffer[256];
	sprintf(buffer,"%i Games on Server:\n",gameCount);
	spFontDrawMiddle( screen->w/4, 2+3*font->maxheight/2, 0, buffer, font );
	spRectangle( screen->w/4, screen->h/2+3*font->maxheight/2-2,0,screen->w/2-10, 18*font->maxheight,65535);
	pGame game = gameList;
	int pos = 0;
	while (game)
	{
		spFontDrawMiddle( screen->w/4, 2+(pos*2+5)*font->maxheight/2, 0, game->name, font );
		game = game->next;
		pos++;
	}
	spFlip();
}

int calc(Uint32 steps)
{
	if (spGetInput()->button[SP_BUTTON_SELECT_NOWASD])
		return 1;
	return 0;
}

void resize( Uint16 w, Uint16 h )
{
	spSelectRenderTarget(screen);
	//Font Loading
	if ( font )
		spFontDelete( font );
	spFontSetShadeColor(0);
	font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 9 * spGetSizeFactor() >> SP_ACCURACY);
	spFontAdd( font, SP_FONT_GROUP_ASCII, 65535 ); //whole ASCII
	spFontAddButton( font, 'R', SP_BUTTON_START_NAME, 65535, SP_ALPHA_COLOR ); //Return == START
	spFontAddButton( font, 'B', SP_BUTTON_SELECT_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); //Backspace == SELECT
	spFontAddButton( font, 'q', SP_BUTTON_L_NAME, 65535, SP_ALPHA_COLOR ); // q == L
	spFontAddButton( font, 'e', SP_BUTTON_R_NAME, 65535, SP_ALPHA_COLOR ); // e == R
	spFontAddButton( font, 'a', SP_BUTTON_LEFT_NAME, 65535, SP_ALPHA_COLOR ); //a == left button
	spFontAddButton( font, 'd', SP_BUTTON_RIGHT_NAME, 65535, SP_ALPHA_COLOR ); // d == right button
	spFontAddButton( font, 'w', SP_BUTTON_UP_NAME, 65535, SP_ALPHA_COLOR ); // w == up button
	spFontAddButton( font, 's', SP_BUTTON_DOWN_NAME, 65535, SP_ALPHA_COLOR ); // s == down button
	spFontMulWidth(font,spFloatToFixed(0.85f));
	spFontAddBorder(font , 0);
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	spSetDefaultWindowSize( 512, 384 );
	spInitCore();
	spInitNet();
	if (connect_to_server())
		return 1;
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	resize( screen->w, screen->h );
	global_counter = 0;
	gameCount = get_games(&gameList);
	//Filling with more debug values
	int i;
	for (i = 0; i < 20; i++)
	{
		pGame newGame = (pGame)malloc(sizeof(tGame));
		sprintf(newGame->name,"Debugspiel %i",i);
		newGame->status = 0;
		newGame->max_player = 20;
		newGame->player_count = i+1;
		newGame->next = gameList;
		newGame->seconds_per_turn = i*5;
		sprintf(newGame->level_string,"todo");
		gameList = newGame;
	}
	spLoop(draw,calc,10,resize,NULL);
	spQuitNet();
	spQuitCore();
	return 0;
}
