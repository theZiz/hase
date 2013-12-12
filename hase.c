#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>

#define LEVEL_WIDTH 1536
#define LEVEL_HEIGHT 1536
#define LEVEL_BORDER 256

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
int help = 0;

int power_pressed = 0;
int direction_pressed = 0;

void loadInformation(char* information)
{
	spClearTarget(0);
	spFontDrawMiddle(screen->w/2,screen->h/2,0,information,font);
	spFlip();
}

#define PHYSIC_IMPACT 13
#include "gravity.c"
#include "player.c"
#include "help.c"
#include "bullet.c"
#include "logic.c"
#include "trace.c"
#include "level.c"

void draw(void)
{
	posX = player[active_player].x;
	posY = player[active_player].y;	
	char buffer[256];
	spClearTarget(0);
	spSetFixedOrign(posX >> SP_ACCURACY,posY >> SP_ACCURACY);
	spSetVerticalOrigin(SP_FIXED);
	spSetHorizontalOrigin(SP_FIXED);
	
	//Level
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,rotation);
	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);

	//Arrow
	if (player[active_player].w_power)
	{
		Sint32 x = spCos(player[active_player].w_direction)*(-8-spFixedToInt(16*player[active_player].w_power));
		Sint32 y = spSin(player[active_player].w_direction)*(-8-spFixedToInt(16*player[active_player].w_power));
		spRotozoomSurface(screen->w/2+(spMul(player[active_player].x-posX+x,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY+y,zoom) >> SP_ACCURACY),0,arrow,spMul(zoom,spGetSizeFactor())/8,spMul(player[active_player].w_power,spMul(zoom,spGetSizeFactor()))/4,player[active_player].w_direction-SP_PI/2);
	}

	//Bullets
	drawBullets();
	
	//Weapon
	spRotozoomSurface(screen->w/2+(spMul(player[active_player].x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY,zoom) >> SP_ACCURACY),0,weapon,zoom/2,zoom/2,player[active_player].w_direction);
	
	//Player
	spSpritePointer sprite = spActiveSprite(player[active_player].hase);
	spSetSpriteZoom(sprite,zoom/2,zoom/2);
	spSetSpriteRotation(sprite,0);
	spDrawSprite(screen->w/2+(spMul(player[active_player].x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY,zoom) >> SP_ACCURACY),0,sprite);

	//Health circle
	spEllipse(screen->w/2+(spMul(player[active_player].x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY-spIntToFixed(14),zoom) >> SP_ACCURACY),0,
	          spFixedToInt(zoom*6)+1,
	          spFixedToInt(zoom*6)+1,spGetRGB(255,0,0));	
	spEllipse(screen->w/2+(spMul(player[active_player].x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY-spIntToFixed(14),zoom) >> SP_ACCURACY),0,
	          spFixedToInt(zoom*6*(player[active_player].health)/MAX_HEALTH)+1,
	          spFixedToInt(zoom*6*(player[active_player].health)/MAX_HEALTH)+1,spGetRGB(0,255,0));
	//spFontDrawMiddle( screen->w/2+(spMul(player[active_player].x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player].y-posY-spIntToFixed(10),zoom) >> SP_ACCURACY)-font->maxheight/2,0,"100", font );
	
	//Not the player:
	int not_active_player = 1-active_player;
	sprite = spActiveSprite(player[not_active_player].hase);
	spSetSpriteZoom(sprite,zoom/2,zoom/2);
	spSetSpriteRotation(sprite,+rotation+player[not_active_player].rotation);
	Sint32 ox = spMul(player[not_active_player].x-posX,zoom);
	Sint32 oy = spMul(player[not_active_player].y-posY,zoom);
	Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
	Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
	spDrawSprite(screen->w/2+x,screen->h/2+y,0,sprite);
	//Health circle
	ox = spMul(player[not_active_player].x-posX-14*-spSin(player[not_active_player].rotation),zoom);
	oy = spMul(player[not_active_player].y-posY-14* spCos(player[not_active_player].rotation),zoom);
	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
	spEllipse(screen->w/2+x,screen->h/2+y,0,
	          spFixedToInt(zoom*6)+1,
	          spFixedToInt(zoom*6)+1,spGetRGB(255,0,0));	
	spEllipse(screen->w/2+x,screen->h/2+y,0,
	          spFixedToInt(zoom*6*(player[not_active_player].health)/MAX_HEALTH)+1,
	          spFixedToInt(zoom*6*(player[not_active_player].health)/MAX_HEALTH)+1,spGetRGB(0,255,0));
	
	//Trace
	drawTrace();
	
	//Help
	draw_help();
	
	//HID
	sprintf(buffer,"FPS: %i",spGetFPS());
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	sprintf(buffer,"Power: %i %%",player[active_player].w_power*100/SP_ONE);
	spFontDraw( 2, 2, 0, buffer, font );
	spFontDrawRight( screen->w-2, 2, 0, "Entry for the Crap\nGame Competition 2013", font );
	int b_alpha = bullet_alpha();
	if (b_alpha)
		spAddColorToTarget(EXPLOSION_COLOR,b_alpha);
	spFlip();
}

int direction_hold = 0;
#define DIRECTION_HOLD_TIME 200

int jump(int high)
{
	Sint32 dx = spSin(player[active_player].rotation);
	Sint32 dy = spCos(player[active_player].rotation);
	if (circle_is_empty(player[active_player].x+dx,player[active_player].y+dy,7))
	{
		if (high)
			player[active_player].hops = HIGH_HOPS_TIME;
		else
			player[active_player].hops = HOPS_TIME;
		player[active_player].high_hops = high;
	}
}

int calc(Uint32 steps)
{
	int i;
	for (i = 0; i < steps; i++)
	{
		Sint32 goal = -player[active_player].rotation;
		if (goal < -SP_PI*3/2 && rotation > -SP_PI/2)
			rotation -= 2*SP_PI;
		if (goal > -SP_PI/2 && rotation < -SP_PI*3/2)
			rotation += 2*SP_PI;
		rotation = rotation*127/128+goal/128;
	}
	update_player(steps);
	if (do_physics(steps))
		return 2;
	counter+=steps;
	if (spGetInput()->button[SP_BUTTON_START])
	{
		spGetInput()->button[SP_BUTTON_START] = 0;
		help = 1-help;
	}
	if (spGetInput()->button[SP_BUTTON_LEFT] && player[active_player].bums && player[active_player].hops <= 0)
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
		if (player[active_player].direction == 1)
		{
			player[active_player].direction = 0;
			player[active_player].w_direction = SP_PI-player[active_player].w_direction;
			direction_hold = 0;
		}
		direction_hold+=steps;
		if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player].bums && player[active_player].hops <= 0)
			jump(0);
	}
	else
	if (spGetInput()->axis[0] > 0)
	{
		if (player[active_player].direction == 0)
		{
			player[active_player].direction = 1;
			player[active_player].w_direction = SP_PI-player[active_player].w_direction;
			direction_hold = 0;
		}
		direction_hold+=steps;
		if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player].bums && player[active_player].hops <= 0)
			jump(0);
	}
	else
		direction_hold = 0;
	
	if (spGetInput()->axis[1] < 0)
	{
		direction_pressed += SP_ONE*steps/20;
		if (direction_pressed >= 128*SP_ONE)
			direction_pressed = 128*SP_ONE;
		if (player[active_player].direction == 0)
			player[active_player].w_direction += (direction_pressed*steps) >> SP_ACCURACY;
		else
			player[active_player].w_direction -= (direction_pressed*steps) >> SP_ACCURACY;
	}
	else
	if (spGetInput()->axis[1] > 0)
	{
		direction_pressed += SP_ONE*steps/20;
		if (direction_pressed >= 128*SP_ONE)
			direction_pressed = 128*SP_ONE;
		if (player[active_player].direction == 0)
			player[active_player].w_direction -= (direction_pressed*steps) >> SP_ACCURACY;
		else
			player[active_player].w_direction += (direction_pressed*steps) >> SP_ACCURACY;
	}
	else
		direction_pressed = 0;

	if (spGetInput()->button[SP_BUTTON_UP])
	{
		power_pressed += SP_ONE*steps/100;
		player[active_player].w_power += (power_pressed*steps) >> SP_ACCURACY;
		if (player[active_player].w_power >= SP_ONE)
			player[active_player].w_power = SP_ONE;
	}
	else
	if (spGetInput()->button[SP_BUTTON_DOWN])
	{
		power_pressed += SP_ONE*steps/100;
		player[active_player].w_power -= (power_pressed*steps) >> SP_ACCURACY;
		if (player[active_player].w_power < 0)
			player[active_player].w_power = 0;
	}
	else
		power_pressed = 0;
	if (spGetInput()->button[SP_BUTTON_RIGHT])
	{
		//Shoot!
		spGetInput()->button[SP_BUTTON_RIGHT] = 0;
		shootBullet(player[active_player].x,player[active_player].y,player[active_player].w_direction+player[active_player].rotation+SP_PI,player[active_player].w_power/2,player[active_player].direction?1:-1);
	}
	updateTrace();
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
	int result = 2;
	while (result == 2)
	{
		loadInformation("Loading images...");
		arrow = spLoadSurface("./data/gravity.png");
		weapon = spLoadSurface("./data/weapon.png");
		bullet = spLoadSurface("./data/bullet.png");
		gravity_surface = spCreateSurface( GRAVITY_DENSITY << GRAVITY_RESOLUTION+1, GRAVITY_DENSITY << GRAVITY_RESOLUTION+1);
		loadInformation("Creating random level...");
		level_original = spCreateSurface(LEVEL_WIDTH,LEVEL_HEIGHT);
		create_level(3,3,3);
		texturize_level();
		loadInformation("Created Arrow image...");
		fill_gravity_surface();
		level = spCreateSurface(LEVEL_WIDTH,LEVEL_HEIGHT);
		loadInformation("Created new surface...");
		level_pixel = (Uint16*)level_original->pixels;
		realloc_gravity();
		init_gravity();
		init_player();
		updateTrace();
		zoomAdjust = spSqrt(spGetSizeFactor());
		minZoom = spSqrt(spGetSizeFactor()/8);
		maxZoom = spSqrt(spGetSizeFactor()*4);
		zoom = spMul(zoomAdjust,zoomAdjust);
		result = spLoop(draw,calc,10,resize,NULL);
		deleteAllBullets();
		free_gravity();
		int i;
		for (i = 0; i < 2; i++)
			spDeleteSpriteCollection(player[i].hase,0);
		spDeleteSurface(arrow);
		spDeleteSurface(weapon);
		spDeleteSurface(bullet);
		spDeleteSurface(level);
		spDeleteSurface(level_original);
		spDeleteSurface(gravity_surface);
	}
	spQuitCore();
	return 0;
}
