#include <sparrow3d.h>
SDL_Surface* screen;
spFontPointer font;
SDL_Surface* testbild;
SDL_Surface* testbild_original;
Sint32* gravity_x = NULL;
Sint32* gravity_y = NULL;
Sint32 counter = 0;

#define DRAW_AT_ONCE 2048
#define GRAVITY_PER_PIXEL (SP_ONE)


void free_gravity()
{
	if (gravity_x)
		free(gravity_x);
	if (gravity_y)
		free(gravity_y);
}

void realloc_gravity()
{
	free_gravity();
	gravity_x = (Sint32*)malloc(sizeof(Sint32)*testbild->w*testbild->h);
	gravity_y = (Sint32*)malloc(sizeof(Sint32)*testbild->w*testbild->h);
}

#define TEST_FIELD 64

Sint32 calc_gravity_x(Uint16* pixel,int x,int y)
{
	Sint32 sum = 0;
	int a,b;
	int a_min = x-TEST_FIELD;
	if (a_min < 0)
		a_min = 0;
	int a_max = x+TEST_FIELD+1;
	if (a_max > testbild->w)
		a_max = testbild->w;
	int b_min = y-TEST_FIELD;
	if (b_min < 0)
		b_min = 0;
	int b_max = y+TEST_FIELD+1;
	if (b_max > testbild->h)
		b_max = testbild->h;
	for (a = a_min; a < a_max; a++)
	{
		int sign = (a<x)?-1:1;
		for (b = b_min; b < b_max; b++)
			if (pixel[a+b*testbild->w] != SP_ALPHA_COLOR)
				sum += sign*GRAVITY_PER_PIXEL/(((a-x)*(a-x)+(b-y)*(b-y)+1));
	}
	return sum;
}

Sint32 calc_gravity_y(Uint16* pixel,int x,int y)
{
	Sint32 sum = 0;
	int a,b;
	int a_min = y-TEST_FIELD;
	if (a_min < 0)
		a_min = 0;
	int a_max = y+TEST_FIELD+1;
	if (a_max > testbild->h)
		a_max = testbild->h;
	int b_min = x-TEST_FIELD;
	if (b_min < 0)
		b_min = 0;
	int b_max = x+TEST_FIELD+1;
	if (b_max > testbild->w)
		b_max = testbild->w;
	for (a = a_min; a < a_max; a++)
	{
		int sign = (a<y)?-1:1;
		for (b = b_min; b < b_max; b++)
			if (pixel[b+a*testbild->w] != SP_ALPHA_COLOR)
				sum += sign*GRAVITY_PER_PIXEL/(((a-y)*(a-y)+(b-x)*(b-x)+1));
	}
	return sum;
}

void update_gravity()
{
	int x,y;
	SDL_LockSurface(testbild);
	SDL_LockSurface(testbild_original);
	Uint16* pixel = (Uint16*)testbild->pixels;
	Uint16* original = (Uint16*)testbild_original->pixels;
	for (x = 0; x < testbild->w; x++)
		for (y = 0; y < testbild->h; y++)
		{
			gravity_x[x+y*testbild->w] = calc_gravity_x(original,x,y);
			gravity_y[x+y*testbild->w] = calc_gravity_y(original,x,y);
			if (pixel[x+y*testbild->w] == SP_ALPHA_COLOR)
				pixel[x+y*testbild->w] = spGetRGB(abs(gravity_x[x+y*testbild->w]) >> 6,abs(gravity_y[x+y*testbild->w]) >> 6,0);
		}
	SDL_UnlockSurface(testbild_original);
	SDL_UnlockSurface(testbild);
}


void draw(void)
{
	srand(0);
	char buffer[256];
	spClearTarget(0);
	Sint32 zoom = spSin(counter*32)+spFloatToFixed(1.25f);
	spRotozoomSurface(screen->w/2,screen->h/2,0,testbild,zoom,zoom,counter*32);
	sprintf(buffer,"FPS: %i",spGetFPS());
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	spFlip();
}

int calc(Uint32 steps)
{
	counter+=steps;
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
	font = spFontLoad( "./data/DejaVuSans-Bold.ttf", 6 * spGetSizeFactor() >> SP_ACCURACY );
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
	spFontAddBorder(font , 0);//spGetRGB(128,128,128));
}

int main(int argc, char **argv)
{
	//srand(time(NULL));
	spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	screen = spCreateDefaultWindow();
	resize( screen->w, screen->h );
	testbild = spLoadSurface("./data/testbild.png");
	testbild_original = spUniqueCopySurface( testbild );
	realloc_gravity();
	update_gravity();
	spSetZSet(0);
	spSetZTest(0);
	spLoop(draw,calc,10,resize,NULL);
	free_gravity();
	spDeleteSurface(testbild);
	spQuitCore();
	return 0;
}
