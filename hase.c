#include <sparrow3d.h>
SDL_Surface* screen;
spFontPointer font;
SDL_Surface* level;
SDL_Surface* level_original;
SDL_Surface* arrow;
Sint32 counter = 0;
int posX,posY;
char levelname[256] = "testlevel";

void loadInformation(char* information)
{
	spClearTarget(0);
	spFontDrawMiddle(screen->w/2,screen->h/2,0,information,font);
	spFlip();
}

#include "gravity.c"
#include "player.c"

void draw(void)
{
	srand(0);
	char buffer[256];
	spClearTarget(0);
	Sint32 zoom = spGetSizeFactor();
	spSetFixedOrign(posX >> SP_ACCURACY,posY >> SP_ACCURACY);
	spSetVerticalOrigin(SP_FIXED);
	spSetHorizontalOrigin(SP_FIXED);
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,0);
	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);
	spSpritePointer sprite = spActiveSprite(hase);
	spSetSpriteZoom(sprite,zoom,zoom);
	spDrawSprite(screen->w/2,screen->h/2,0,sprite);
	//spEllipseBorder(screen->w/2,screen->h/2,0,32,32,1,1,spGetRGB(255,0,0));
	sprintf(buffer,"FPS: %i",spGetFPS());
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	spFlip();
}

int calc(Uint32 steps)
{
	spUpdateSprite(spActiveSprite(hase),steps);
	counter+=steps;
	if (spGetInput()->axis[0] < 0)
		posX-=steps*8192*2;
	if (spGetInput()->axis[0] > 0)
		posX+=steps*8192*2;
	if (spGetInput()->axis[1] < 0)
		posY-=steps*8192*2;
	if (spGetInput()->axis[1] > 0)
		posY+=steps*8192*2;
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
	font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 12 * spGetSizeFactor() >> SP_ACCURACY);
	spFontAdd( font, SP_FONT_GROUP_ASCII, 65535 ); //whole ASCII
	spFontAddButton( font, 'R', SP_BUTTON_START_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); //Return == START
	spFontAddButton( font, 'B', SP_BUTTON_SELECT_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); //Backspace == SELECT
	spFontAddButton( font, 'q', SP_BUTTON_L_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); // q == L
	spFontAddButton( font, 'e', SP_BUTTON_R_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); // e == R
	spFontAddButton( font, 'a', SP_BUTTON_LEFT_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); //a == left button
	spFontAddButton( font, 'd', SP_BUTTON_RIGHT_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); // d == right button
	spFontAddButton( font, 'w', SP_BUTTON_UP_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); // w == up button
	spFontAddButton( font, 's', SP_BUTTON_DOWN_NOWASD_NAME, 65535, SP_ALPHA_COLOR ); // s == down button
	spFontMulWidth(font,spFloatToFixed(0.85f));
	spFontAddBorder(font , 0);
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	resize( screen->w, screen->h );
	loadInformation("Loading images...");
	char buffer[256];
	sprintf(buffer,"./levels/%s.png",levelname);
	level_original = spLoadSurface(buffer);
	posX = level_original->w << SP_ACCURACY-1;
	posY = level_original->h << SP_ACCURACY-1;
	arrow = spLoadSurface("./data/gravity.png");
	hase = spLoadSpriteCollection("./data/hase.ssc",NULL);
	gravity_surface = spCreateSurface( GRAVITY_DENSITY << GRAVITY_RESOLUTION+1, GRAVITY_DENSITY << GRAVITY_RESOLUTION+1);
	loadInformation("Created Arrow image...");
	fill_gravity_surface();
	level = spCreateSurface(level_original->w,level_original->h);
	realloc_gravity();
	init_gravity();
	init_player();
	spLoop(draw,calc,10,resize,NULL);
	free_gravity();
	spDeleteSpriteCollection(hase,0);
	spDeleteSurface(arrow);
	spDeleteSurface(level);
	spDeleteSurface(level_original);
	spDeleteSurface(gravity_surface);
	spQuitCore();
	return 0;
}
