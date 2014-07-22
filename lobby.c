#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "lobbyList.h"

SDL_Surface* screen;
spFontPointer font;

void resize( Uint16 w, Uint16 h )
{
	spSelectRenderTarget(screen);
	//Font Loading
	if ( font )
		spFontDelete( font );
	spFontSetShadeColor(0);
	font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 7 * spGetSizeFactor() >> SP_ACCURACY);
	spFontAdd( font, SP_FONT_GROUP_ASCII"™", 65535 ); //whole ASCII
	spFontAddBorder(font , 0);
	spFontAddButton( font, 'R', SP_BUTTON_START_NAME, 65535, spGetRGB(127,127,127) ); //Return == START
	spFontAddButton( font, 'B', SP_BUTTON_SELECT_NAME, 65535, spGetRGB(127,127,127) ); //Backspace == SELECT
	spFontAddButton( font, 'q', SP_BUTTON_L_NAME, 65535, spGetRGB(127,127,127) ); // q == L
	spFontAddButton( font, 'e', SP_BUTTON_R_NAME, 65535, spGetRGB(127,127,127) ); // e == R
	spFontAddButton( font, 'a', SP_BUTTON_LEFT_NAME, 65535, spGetRGB(127,127,127) ); //a == left button
	spFontAddButton( font, 'd', SP_BUTTON_RIGHT_NAME, 65535, spGetRGB(127,127,127) ); // d == right button
	spFontAddButton( font, 'w', SP_BUTTON_UP_NAME, 65535, spGetRGB(127,127,127) ); // w == up button
	spFontAddButton( font, 's', SP_BUTTON_DOWN_NAME, 65535, spGetRGB(127,127,127) ); // s == down button
	spFontMulWidth(font,spFloatToFixed(0.9f));
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	spSetDefaultWindowSize( 320, 240 );
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
