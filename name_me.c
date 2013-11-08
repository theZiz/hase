#include <sparrow3d.h>
SDL_Surface* screen;
spFontPointer font;
SDL_Surface* level;
SDL_Surface* level_original;
SDL_Surface* gravity_surface;
typedef struct {
	Sint32 mass,x,y;
} tGravity;
#define GRAVITY_RESOLUTION 3
#define GRAVITY_PER_PIXEL (SP_ONE/196)
tGravity* gravity;
Sint32 counter = 0;


void free_gravity()
{
	if (gravity)
		free(gravity);
}

void realloc_gravity()
{
	free_gravity();
	gravity = (tGravity*)malloc(sizeof(tGravity)*level->w*level->h>>2*GRAVITY_RESOLUTION);
}

Sint32 calc_mass(Uint16* original,int x,int y)
{
	Sint32 mass = 0;
	int a,b;
	for (a = 0; a < (1<<GRAVITY_RESOLUTION); a++)	
		for (b = 0; b < (1<<GRAVITY_RESOLUTION); b++)
		{
			if (original[(x<<GRAVITY_RESOLUTION)+a+((y<<GRAVITY_RESOLUTION)+b)*level->w] != SP_ALPHA_COLOR )
				mass+=GRAVITY_PER_PIXEL;
		}
	return mass;
}

#define GRAVITY_CIRCLE 12

void impact_gravity(Sint32 mass,int x,int y)
{
	int a,b;
	int start_a = x-GRAVITY_CIRCLE;
	if (start_a < 0)
		start_a = 0;
	int end_a = x+GRAVITY_CIRCLE+1;
	if (end_a > (level->w>>GRAVITY_RESOLUTION))
		end_a = (level->w>>GRAVITY_RESOLUTION);
	int start_b = y-GRAVITY_CIRCLE;
	if (start_b < 0)
		start_b = 0;
	int end_b = y+GRAVITY_CIRCLE+1;
	if (end_b > (level->h>>GRAVITY_RESOLUTION))
		end_b = (level->h>>GRAVITY_RESOLUTION);
	for (a = start_a; a < end_a; a++)	
		for (b = start_b; b < end_b; b++)
		{
			Sint32 dx = a-x;
			Sint32 dy = b-y;
			Sint32 sum = spFixedToInt(spSqrt(spIntToFixed(dx*dx+dy*dy+1)));//abs(dx)+abs(dy)+1;
			gravity[a+b*(level->w>>GRAVITY_RESOLUTION)].x += dx*mass/sum;
			gravity[a+b*(level->w>>GRAVITY_RESOLUTION)].y += dy*mass/sum;
		}
}

void update_gravity()
{
	int x,y;
	SDL_LockSurface(level_original);
	Uint16* original = (Uint16*)level_original->pixels;
	memset(gravity,0,sizeof(tGravity)*level->w*level->h>>2*GRAVITY_RESOLUTION);
	for (x = 0; x < (level->w>>GRAVITY_RESOLUTION); x++)
		for (y = 0; y < (level->h>>GRAVITY_RESOLUTION); y++)
		{
			gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass = calc_mass(original,x,y);
			if (gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass)
				impact_gravity(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass,x,y);
		}
	SDL_UnlockSurface(level_original);
	spSelectRenderTarget(level);
	spSetAlphaTest(1);
	for (x = 0; x < (level->w>>GRAVITY_RESOLUTION); x++)
	{
		for (y = 0; y < (level->h>>GRAVITY_RESOLUTION); y++)
		{
			Sint32 force = spSqrt(spMul(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].x,
			                      gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].x)+
			                      spMul(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y,
			                      gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y));			                      
			int f = 31-force / GRAVITY_PER_PIXEL / 512;
			if (f < 0)
				f = 0;
			Sint32 angle = 0;
			if (force)
			{
				Sint32 ac = spDiv(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y,force);
				if (ac < -SP_ONE)
					ac = -SP_ONE;
				if (ac > SP_ONE)
					ac = SP_ONE;
				angle = spAcos(ac)*16/SP_PI;
			}
			if (x <= 0)
				angle = 31-angle;
			if (angle > 31)
				angle = 31;
			if (angle < 0)
				angle = 0;
			spBlitSurfacePart(x<<GRAVITY_RESOLUTION,y<<GRAVITY_RESOLUTION,0,
			                  gravity_surface,angle*8,f*8,8,8);
		}
	}
	spSetBlending(SP_ONE/2);
	spBlitSurface(level->w/2,level->h/2,0,level_original);
	spSetBlending(SP_ONE);
	spSelectRenderTarget(screen);
}


void draw(void)
{
	srand(0);
	char buffer[256];
	spClearTarget(0);
	Sint32 zoom = spSin(counter*32)+spFloatToFixed(1.25f);
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,counter*32);
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
	srand(time(NULL));
	//spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	screen = spCreateDefaultWindow();
	spSetZSet(0);
	spSetZTest(0);
	resize( screen->w, screen->h );
	level_original = spLoadSurface("./data/testbild.png");
	gravity_surface = spLoadSurface("./data/gravity.png");
	level = spCreateSurface(level_original->w,level_original->h);
	realloc_gravity();
	update_gravity();
	spLoop(draw,calc,10,resize,NULL);
	free_gravity();
	spDeleteSurface(level);
	spDeleteSurface(gravity_surface);
	spQuitCore();
	return 0;
}
