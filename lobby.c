#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "lobbyList.h"

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
}

int main(int argc, char **argv)
{
	spSetRand(time(NULL));
	spSetDefaultWindowSize( 512, 384 );
	spInitCore();
	spInitNet();
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	resize( screen->w, screen->h );
	start_lobby(font,resize);
	spQuitNet();
	spQuitCore();
	return 0;
}
