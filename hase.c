#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>
SDL_Surface* screen;
spFontPointer font;
SDL_Surface* level;
SDL_Surface* level_original;
Uint16* level_pixel;
SDL_Surface* arrow;
SDL_Surface* weapon;
SDL_Surface* bullet;
Sint32 counter = 0;
int posX,posY,rotation;
Sint32 zoom;
Sint32 zoomAdjust;
Sint32 minZoom,maxZoom;
char levelname[256] = "testlevel2";
int help = 0;

void loadInformation(char* information)
{
	spClearTarget(0);
	spFontDrawMiddle(screen->w/2,screen->h/2,0,information,font);
	spFlip();
}

int map_follows = 1;

#include "gravity.c"
#include "player.c"
#include "help.c"
#include "bullet.c"
#include "logic.c"

void draw(void)
{
	srand(0);
	char buffer[256];
	spClearTarget(0);
	spSetFixedOrign(posX >> SP_ACCURACY,posY >> SP_ACCURACY);
	spSetVerticalOrigin(SP_FIXED);
	spSetHorizontalOrigin(SP_FIXED);
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,rotation);
	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);
	spSpritePointer sprite = spActiveSprite(hase);
	
	posX = player.x;
	posY = player.y;
	
	spSetSpriteZoom(sprite,zoom,zoom);
	if (map_follows)
	{
		spRotozoomSurface(screen->w/2+(spMul(player.x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player.y-posY,zoom) >> SP_ACCURACY),0,weapon,zoom,zoom,player.w_direction);
		if (player.w_power)
		{
			Sint32 x = spCos(player.w_direction)*(-16-spFixedToInt(8*player.w_power));
			Sint32 y = spSin(player.w_direction)*(-16-spFixedToInt(8*player.w_power));
			spRotozoomSurface(screen->w/2+(spMul(player.x-posX+x,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player.y-posY+y,zoom) >> SP_ACCURACY),0,arrow,spMul(zoom,spGetSizeFactor())/8,spMul(player.w_power,spMul(zoom,spGetSizeFactor()))/8,player.w_direction-SP_PI/2);
		}
		spSetSpriteRotation(sprite,0);
		spDrawSprite(screen->w/2+(spMul(player.x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player.y-posY,zoom) >> SP_ACCURACY),0,sprite);
	}
	else
	{
		spRotozoomSurface(screen->w/2,screen->h/2,0,weapon,zoom,zoom,player.rotation+player.w_direction);
		if (player.w_power)
		{
			int x = spFixedToInt(spCos(player.rotation+player.w_direction)*spFixedToInt((-16-spFixedToInt(8*player.w_power))*zoom));
			int y = spFixedToInt(spSin(player.rotation+player.w_direction)*spFixedToInt((-16-spFixedToInt(8*player.w_power))*zoom));
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,arrow,zoom/4,spMul(player.w_power,spMul(zoom,spGetSizeFactor()))/8,player.rotation+player.w_direction-SP_PI/2);
		}
		spSetSpriteRotation(sprite,player.rotation);
		spDrawSprite(screen->w/2,screen->h/2,0,sprite);
	}
	//spEllipseBorder(screen->w/2,screen->h/2,0,32,32,1,1,spGetRGB(255,0,0));
	drawBullets();
	draw_help();
	sprintf(buffer,"FPS: %i",spGetFPS());
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	spFlip();
}

int direction_hold = 0;
#define DIRECTION_HOLD_TIME 200

int jump(int high)
{
	Sint32 dx = spSin(player.rotation);
	Sint32 dy = spCos(player.rotation);
	if (circle_is_empty(player.x+dx,player.y+dy,15))
	{
		if (high)
			player.hops = HIGH_HOPS_TIME;
		else
			player.hops = HOPS_TIME;
		player.high_hops = high;
	}
}

int calc(Uint32 steps)
{
	int i;
	if (map_follows)
		for (i = 0; i < steps; i++)
		{
			Sint32 goal = -player.rotation;
			if (goal < -SP_PI*3/2 && rotation > -SP_PI/2)
				rotation -= 2*SP_PI;
			if (goal > -SP_PI/2 && rotation < -SP_PI*3/2)
				rotation += 2*SP_PI;
			rotation = rotation*127/128+goal/128;
		}
	else
		rotation = 0;
	update_player(steps);
	do_physics(steps);
	counter+=steps;
	if (spGetInput()->button[SP_BUTTON_START])
	{
		spGetInput()->button[SP_BUTTON_START] = 0;
		help = 1-help;
	}
	if (spGetInput()->button[SP_BUTTON_LEFT] && player.bums && player.hops <= 0)
	{
		jump(0);
	}
	if (spGetInput()->button[SP_BUTTON_RIGHT] && player.bums && player.hops <= 0)
	{
		jump(1);
	}
	if (spGetInput()->button[SP_BUTTON_L])
	{
		zoomAdjust -= steps*32;
		if (zoomAdjust < minZoom)
			zoomAdjust = minZoom;
		zoom = spMul(zoomAdjust,zoomAdjust);
	}
	if (spGetInput()->button[SP_BUTTON_R])
	{
		zoomAdjust += steps*32;
		if (zoomAdjust > maxZoom)
			zoomAdjust = maxZoom;
		zoom = spMul(zoomAdjust,zoomAdjust);
	}
	update_player_sprite(steps);
	if (spGetInput()->axis[0] < 0)
	{
		if (player.direction == 1)
		{
			player.direction = 0;
			player.w_direction = SP_PI-player.w_direction;
			direction_hold = 0;
		}
		direction_hold+=steps;
		if (direction_hold >= DIRECTION_HOLD_TIME && player.bums && player.hops <= 0)
			jump(0);
	}
	else
	if (spGetInput()->axis[0] > 0)
	{
		if (player.direction == 0)
		{
			player.direction = 1;
			player.w_direction = SP_PI-player.w_direction;
			direction_hold = 0;
		}
		direction_hold+=steps;
		if (direction_hold >= DIRECTION_HOLD_TIME && player.bums && player.hops <= 0)
			jump(0);
	}
	else
		direction_hold = 0;
	if (spGetInput()->axis[1] < 0)
	{
		if (player.direction == 0)
			player.w_direction+=steps*128;
		else
			player.w_direction-=steps*128;
	}
	if (spGetInput()->axis[1] > 0)
	{
		if (player.direction == 0)
			player.w_direction-=steps*128;
		else
			player.w_direction+=steps*128;
	}
	if (spGetInput()->button[SP_BUTTON_UP])
	{
		player.w_power += steps*64;
		if (player.w_power >= 2*SP_ONE)
			player.w_power = 2*SP_ONE;
	}
	else
	if (player.w_power)
	{
		//Shoot!
		shootBullet(player.x,player.y,player.w_direction+player.rotation+SP_PI,player.w_power/2);
		player.w_power = 0;
	}
	/*if (spGetInput()->button[SP_BUTTON_START])
	{
		spGetInput()->button[SP_BUTTON_START] = 0;
		map_follows = 1-map_follows;
	}*/
		
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
	arrow = spLoadSurface("./data/gravity.png");
	weapon = spLoadSurface("./data/weapon.png");
	bullet = spLoadSurface("./data/bullet.png");
	hase = spLoadSpriteCollection("./data/hase.ssc",NULL);
	gravity_surface = spCreateSurface( GRAVITY_DENSITY << GRAVITY_RESOLUTION+1, GRAVITY_DENSITY << GRAVITY_RESOLUTION+1);
	loadInformation("Created Arrow image...");
	fill_gravity_surface();
	level = spCreateSurface(level_original->w,level_original->h);
	loadInformation("Created new surface...");
	level_pixel = (Uint16*)level_original->pixels;
	realloc_gravity();
	init_gravity();
	init_player();
	zoomAdjust = spSqrt(spGetSizeFactor());
	minZoom = spSqrt(spGetSizeFactor()/8);
	maxZoom = spSqrt(spGetSizeFactor()*4);
	zoom = spMul(zoomAdjust,zoomAdjust);
	spLoop(draw,calc,10,resize,NULL);
	free_gravity();
	spDeleteSpriteCollection(hase,0);
	spDeleteSurface(arrow);
	spDeleteSurface(weapon);
	spDeleteSurface(bullet);
	spDeleteSurface(level);
	spDeleteSurface(level_original);
	spDeleteSurface(gravity_surface);
	spQuitCore();
	return 0;
}
