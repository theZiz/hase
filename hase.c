#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>

#include "lobbyList.h"
#include "hase.h"
#include "options.h"
#ifdef PROFILE
	int calc_time = 0;
	int draw_time = 0;
#endif

pGame hase_game;
pPlayer hase_player_list = NULL;

#define LEVEL_WIDTH 1536
#define LEVEL_HEIGHT 1536

spSound* snd_explosion;
spSound* snd_end;
int channel_end;
spSound* snd_beep;
spSound* snd_high;
spSound* snd_low;
spSound* snd_shoot;
spSound* snd_turn;

spFontPointer font;
spFontPointer font_dark;
SDL_Surface* level;
SDL_Surface* level_original;
Uint16* level_pixel;
SDL_Surface* arrow;
int posX,posY,rotation;
Sint32 zoom;
Sint32 zoom_d; 
Sint32 zoomAdjust;
Sint32 minZoom,maxZoom;
int help = 0;
int countdown;

int power_pressed = 0;
int direction_pressed = 0;
int alive_count;

int game_pause = 0;
int extra_time = 0;
int wp_choose = 0;

int map_w,map_h,map_size;

int speed = 1;
int winter;

#define INPUT_AXIS_0_LEFT 0
#define INPUT_AXIS_0_RIGHT 1
#define INPUT_AXIS_1_LEFT 2
#define INPUT_AXIS_1_RIGHT 3
#define INPUT_BUTTON_OK 4
#define INPUT_BUTTON_CANCEL 5
#define INPUT_BUTTON_3 6
#define INPUT_BUTTON_4 7
#define INPUT_BUTTON_L 8
#define INPUT_BUTTON_R 9
#define INPUT_BUTTON_START 10
#define INPUT_BUTTON_SELECT 11
int input_states[12];
char button_states[8];
unsigned char send_data[1536];
spParticleBunchPointer particles = NULL;

spSpriteCollectionPointer targeting;

spNetIRCMessagePointer before_showing;

SDL_Surface* screen;
SDL_Surface* tomato;

void ( *hase_resize )( Uint16 w, Uint16 h );

void loadInformation(char* information)
{
	spClearTarget(0);
	spFontDrawMiddle(screen->w/2,screen->h/2,0,information,font);
	spFlip();
}

#define PHYSIC_IMPACT 13

#define ITEMS_COUNT 3
typedef struct sItem *pItem;
typedef struct sItem
{
	int kind;
	Sint32 x,y;
	Sint32 dx,dy;
	Sint32 rotation;
	int seen;
	int beep;
	pItem next;
} tItem;
pItem items_drop(int kind,Sint32 x,Sint32 y);
void update_map();
#include "gravity.c"
#include "player.c"
#include "help.c"
#include "bullet.c"
#include "logic.c"
#include "trace.c"
#include "items.c"
#include "level.h"

char chatMessage[256];
pWindow chatWindow;

SDL_Surface* map_surface;

void draw_map(int px,int py)
{
	spRotozoomSurface(px,py,0,map_surface,SP_ONE,SP_ONE,rotation);	
	//player
	int j;
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->firstHare == NULL)
			continue;
		pHare hare = player[j]->firstHare;
		if (hare)
		do
		{	
			int X = hare->x-(LEVEL_WIDTH << SP_ACCURACY-1);
			int Y = hare->y-(LEVEL_HEIGHT << SP_ACCURACY-1);
			int x = px+(spMul(spCos(rotation),X) - spMul(spSin(rotation),Y) >> SP_ACCURACY)*map_w/LEVEL_WIDTH;
			int y = py+(spMul(spSin(rotation),X) + spMul(spCos(rotation),Y) >> SP_ACCURACY)*map_h/LEVEL_HEIGHT;
			if (j == active_player)
			{
				if (hare == player[j]->activeHare)
					spEllipse(x,y,0,map_size,map_size,spGetFastRGB(0,255,255));
				else
					spEllipse(x,y,0,map_size,map_size,spGetFastRGB(0,255,0));
			}
			else
				spEllipse(x,y,0,map_size,map_size,spGetFastRGB(255,0,0));
			hare = hare->next;
		}
		while (hare != player[j]->firstHare);	
	}
	pItem item = firstItem;
	while (item)
	{
		int X = item->x-(LEVEL_WIDTH << SP_ACCURACY-1);
		int Y = item->y-(LEVEL_HEIGHT << SP_ACCURACY-1);
		int x = px+(spMul(spCos(rotation),X) - spMul(spSin(rotation),Y) >> SP_ACCURACY)*map_w/LEVEL_WIDTH;
		int y = py+(spMul(spSin(rotation),X) + spMul(spCos(rotation),Y) >> SP_ACCURACY)*map_h/LEVEL_HEIGHT;
		switch (item->kind)
		{
			case 0:
				spRectangle(x,y,0,map_size,map_size,spGetFastRGB(255,255,255));
				break;
			case 1:
				spRectangle(x,y,0,map_size,map_size,spGetFastRGB(255,255,0));
				break;
			case 2:
				spRectangle(x,y,0,map_size,map_size,spGetFastRGB(100,50,0));
				break;
		}
		item = item->next;
	}
	pBullet bullet = firstBullet;
	while (bullet)
	{
		int X = bullet->x-(LEVEL_WIDTH << SP_ACCURACY-1);
		int Y = bullet->y-(LEVEL_HEIGHT << SP_ACCURACY-1);
		int x = px+(spMul(spCos(rotation),X) - spMul(spSin(rotation),Y) >> SP_ACCURACY)*map_w/LEVEL_WIDTH;
		int y = py+(spMul(spSin(rotation),X) + spMul(spCos(rotation),Y) >> SP_ACCURACY)*map_h/LEVEL_HEIGHT;
		spEllipse(x,y,0,map_size,map_size,spGetFastRGB(127,127,255));
		bullet = bullet->next;
	}
}

typedef struct sChatLine *pChatLine;
typedef struct sChatLine
{
	char line[1024];
	spFontPointer font;
	int t;
	pChatLine next;
} tChatLine;

pChatLine firstChatLine = NULL;
pChatLine lastChatLine = NULL;
int chatLineCount = 0;

void draw(void)
{
	#ifdef PROFILE
		int start_time = SDL_GetTicks();
	#endif
	char buffer[4096]; //Nobody will ever use more than 4096 byte :P
	spClearTarget(0);
	spSetFixedOrign(posX >> SP_ACCURACY,posY >> SP_ACCURACY);
	spSetVerticalOrigin(SP_FIXED);
	spSetHorizontalOrigin(SP_FIXED);
	if (gop_rotation() == 0)
		rotation = 0;
	
	//Level
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,rotation);
	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);
	
	//Items
	items_draw();

	//Players:
	int j;
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->firstHare == NULL)
			continue;
		pHare hare = player[j]->firstHare;
		if (hare)
		do
		{							
			spSpritePointer sprite = spActiveSprite(hare->hase);
			spSetSpriteZoom(sprite,zoom/2,zoom/2);
			spSetSpriteRotation(sprite,+rotation+hare->cam_rotation);
			Sint32 ox = spMul(hare->x-posX,zoom);
			Sint32 oy = spMul(hare->y-posY,zoom);
			Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
			Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
			if (j == active_player && hare == player[j]->activeHare)
			{
				if (gop_circle())
				{
					spSetBlending( SP_ONE/2 );
					int r = zoom*3 >> SP_ACCURACY-2;
					spEllipse(screen->w/2+x,screen->h/2+y,0,r,r,65535);
					spSetBlending( SP_ONE );
				}
				//Weapon
				//spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,weapon_surface[hare->wp_y][hare->wp_x],zoom/2,zoom/2,hare->w_direction+rotation+hare->rotation);
				int w_nr = weapon_pos[player[active_player]->activeHare->wp_y][player[active_player]->activeHare->wp_x];
				//building
				if (w_nr == WP_BUILD_SML || w_nr == WP_BUILD_MID || w_nr == WP_BUILD_BIG)
				{
					
					int r = (zoom*weapon_explosion[w_nr] >> SP_ACCURACY+1);
					int d = 12+weapon_explosion[w_nr]+(hare->w_power*(12+weapon_explosion[w_nr]) >> SP_ACCURACY);
					Sint32 ox = spMul(hare->x-posX-d*-spMul(spSin(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32 oy = spMul(hare->y-posY-d* spMul(spCos(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
					Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;

					ox = hare->x-d*-spMul(spSin(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3);
					oy = hare->y-d* spMul(spCos(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3);

					spSetBlending( SP_ONE*2/3 );
					if (circle_is_empty(ox>>SP_ACCURACY,oy>>SP_ACCURACY,weapon_explosion[w_nr]/2+4,NULL,-1))
						spEllipse(screen->w/2+x,screen->h/2+y,0,r,r,spGetFastRGB(0,255,0));
					else
						spEllipse(screen->w/2+x,screen->h/2+y,0,r,r,spGetFastRGB(255,0,0));
					spSetBlending( SP_ONE );
				}
				else
				//Arrow
				{
					Sint32 w_zoom = spMax(SP_ONE/2,zoom);
					spSpritePointer target = spActiveSprite(targeting);
					spSetSpriteZoom(target,w_zoom,w_zoom);
					Sint32 ox = spMul(hare->x-posX,zoom)-spMul(20*-spSin(hare->rotation+hare->w_direction-SP_PI/2),w_zoom);
					Sint32 oy = spMul(hare->y-posY,zoom)-spMul(20* spCos(hare->rotation+hare->w_direction-SP_PI/2),w_zoom);
					Sint32 nx = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
					Sint32 ny = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
					//spSetBlending( SP_ONE*2/3 );
					spDrawSprite(screen->w/2+nx,screen->h/2+ny,0,target);
					spLine(screen->w/2+x,screen->h/2+y,0,screen->w/2+nx,screen->h/2+ny,0,get_border_color());
					//spSetBlending( SP_ONE );
				}
				/*if (hare->w_power)
				{
					Sint32 w_zoom = spMax(SP_ONE,zoom);
					Sint32 ox = spMul(hare->x-posX-14*-spMul(spSin(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32 oy = spMul(hare->y-posY-14* spMul(spCos(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
					Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
					spSetBlending( SP_ONE*2/3 );
					spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,arrow,spMul(w_zoom,spGetSizeFactor())/16,spMul(hare->w_power,spMul(w_zoom,spGetSizeFactor()))/4,hare->w_direction+rotation+hare->rotation-SP_PI/2);
					spSetBlending( SP_ONE );
				}*/
				
			}
			spDrawSprite(screen->w/2+x,screen->h/2+y,0,sprite);
			//Health bar
			y-=zoom*3>>14;
			spSetBlending( SP_ONE*2/3 );
			spRectangle( screen->w/2+x,screen->h/2+y,0,zoom >> 12,zoom >> 15,spGetRGB(255,0,0));
			spSetBlending( SP_ONE );
			spRectangle( screen->w/2+x-((MAX_HEALTH-hare->health)*zoom/MAX_HEALTH >> 13),screen->h/2+y,0,
				hare->health*zoom/MAX_HEALTH >> 12,zoom >> 15,spGetRGB(0,255,0));
			//labels
			y-=zoom>>15;
			if (player[j]->computer)
				sprintf(buffer,"%s (AI)",player[j]->name);
			else
				sprintf(buffer,"%s",player[j]->name);
			spSetBlending( SP_ONE*2/3 );
			if (gop_show_names())
				spFontDrawMiddle( screen->w/2+x,screen->h/2+y-font->maxheight,0,buffer, font );
			if (j == active_player && player[j]->computer && ai_shoot_tries>1 && player[j]->weapon_points && hare == player[j]->activeHare)
			{
				sprintf(buffer,"Aiming (%2i%%)",ai_shoot_tries*100/AI_MAX_TRIES);
				spFontDrawMiddle( screen->w/2+x,screen->h/2+y-(1+gop_show_names())*font->maxheight,0,buffer, font );
			}
			if (j == active_player && player[j]->kicked == 2 && hare == player[j]->activeHare)
			{
				sprintf(buffer,"Zombie (Disconnected)");
				spFontDrawMiddle( screen->w/2+x,screen->h/2+y-(1+gop_show_names())*font->maxheight,0,buffer, font );
			}
			spSetBlending( SP_ONE );
			hare = hare->next;
		}
		while (hare != player[j]->firstHare);
	}
	//Particles
	spParticleDraw(particles);
	
	//Bullets
	drawBullets();
		
	//Trace
	drawTrace(player[active_player]);
		
	//Map
	if (gop_show_map())
		draw_map(screen->w-map_w/2,map_h/2);
	
	//HID
	int y = 0;
	if (get_channel())
	{
		time_t now = time(NULL) - 60;
		pChatLine line = firstChatLine;
		while (line)
		{
			int diff = line->t - now;
			if (diff < 16)
				spSetBlending(diff * SP_ONE/16);
			spFontDraw(2, y-line->font->maxheight/8, 0, line->line, line->font );
			y += line->font->maxheight*3/4+(spGetSizeFactor()*2 >> SP_ACCURACY);
			spSetBlending(SP_ONE);
			line = line->next;
		}
	}
	
	y = screen->h - alive_count * (font->maxheight*3/4+(spGetSizeFactor()*2 >> SP_ACCURACY));
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->firstHare == NULL)
			continue;
		int health = 0;
		int count = 0;
		pHare hare = player[j]->firstHare;
		do
		{
			health += hare->health;
			count++;
			hare = hare->next;
		}
		while (hare != player[j]->firstHare);
		
		int w = health*screen->w/(hase_game->hares_per_player*MAX_HEALTH*5);
		if  (j != active_player)
			spSetPattern8(153,60,102,195,153,60,102,195);
		spRectangle(w/2, y+font->maxheight*3/8,0,w,font->maxheight*3/4,spSpriteAverageColor(hare->hase->active));
		spDeactivatePattern();

		sprintf(buffer,"%i/%i",health,hase_game->hares_per_player*MAX_HEALTH);
		int width = spFontWidth(buffer,font);
		int pos_x = w/2-width/2;
		if (pos_x < 0)
			pos_x = 0;
		spFontDraw(pos_x,y-font->maxheight/8,0,buffer,font);
		w = spMax(w,width);
		sprintf(buffer,"%s (%i)",player[j]->name,count);
		w += spFontDraw(w+2, y-font->maxheight/8, 0, buffer, font );

		if (player[j]->d_health)
		{
			if (player[j]->d_health < 0)
				sprintf(buffer," %i",player[j]->d_health);
			else
				sprintf(buffer," -%i",player[j]->d_health);
			spFontDraw(w+2,y+font->maxheight/2*(spGetSizeFactor() > SP_ONE?4:3)/4-font->maxheight/2,0,buffer,font);
		}

		y += font->maxheight*3/4+(spGetSizeFactor()*2 >> SP_ACCURACY);
	}
		
	spFontDrawRight( screen->w-1 - player[active_player]->weapon_points * tomato->w, screen->h-1-font->maxheight, 0, 
		player[active_player]->weapon_points > 0 ? "Action points:" : "Action points: None", font );
	int i;
	for (i = 0; i < player[active_player]->weapon_points; i++)
		spBlitSurface( screen->w-1 - tomato->w*(2*i+1)/2, screen->h-1-font->maxheight/2,0,tomato);
	if (player[active_player]->activeHare)
	{
		int w_nr = weapon_pos[player[active_player]->activeHare->wp_y][player[active_player]->activeHare->wp_x];
		spFontDrawRight( screen->w-1, screen->h-1-font->maxheight*2, 0, weapon_name[w_nr], font );
		if (w_nr == WP_BUILD_SML || w_nr == WP_BUILD_MID || w_nr == WP_BUILD_BIG)
			sprintf(buffer,"Distance: %i",player[active_player]->activeHare->w_power*30/SP_ONE+30);
		else
			sprintf(buffer,"Power: %i.%i %%",player[active_player]->activeHare->w_power*100/SP_ONE,player[active_player]->activeHare->w_power*1000/SP_ONE%10);
		spFontDrawRight( screen->w-1, screen->h-1-3*font->maxheight, 0, buffer, font );
	}
	if (player[active_player]->weapon_points)
		sprintf(buffer,"%is",countdown / 1000);
	else
	if (extra_time)
		sprintf(buffer,"%is",extra_time / 1000);
	else
		sprintf(buffer,"∞");
	if (speed > 1)
		sprintf(&buffer[strlen(buffer)]," (>> x%i)",speed);
	spFontDrawMiddle( screen->w >> 1, screen->h-1-font->maxheight, 0, buffer, font );

	
	if (wp_choose)
		draw_weapons();

	//Help
	draw_help();

	int b_alpha = bullet_alpha();
	if (b_alpha)
		spAddColorToTarget(EXPLOSION_COLOR,b_alpha);
	
	//Error message
	if (game_pause)
		draw_message();

	spMapSetMapSet(0);
	if (chatWindow)
		window_draw();
	spMapSetMapSet(1);
		
	#ifdef PROFILE
		draw_time = SDL_GetTicks() - start_time;
		sprintf(buffer,"FPS: %i\nDraw: %ims\nCalc: %ims",spGetFPS(),draw_time,calc_time);
		spFontDrawMiddle( screen->w >> 1, 1, 0, buffer, font );
	#endif
	
	spFlip();
}

int direction_hold = 0;
#define DIRECTION_HOLD_TIME 200

void jump(int high)
{
	if (player[active_player]->activeHare == NULL)
		return;
	//if (circle_is_empty(player[active_player]->x+dx >> SP_ACCURACY,player[active_player]->y+dy >> SP_ACCURACY,6,0xDEAD))
	{
		if (high)
			player[active_player]->activeHare->hops = HIGH_HOPS_TIME;
		else
			player[active_player]->activeHare->hops = HOPS_TIME;
		player[active_player]->activeHare->high_hops = high;
	}
}

int min_d_not_me(int x,int y,int me)
{
	int min_d = LEVEL_WIDTH*LEVEL_WIDTH;
	int i;
	for (i = 0; i < player_count;i++)
	{
		if (player[i]->firstHare == NULL)
			continue;
		pHare hare = player[i]->firstHare;
		if (hare)
		do
		{							
			int d = spFixedToInt(hare->x-x)*spFixedToInt(hare->x-x)+
					spFixedToInt(hare->y-y)*spFixedToInt(hare->y-y);
			if (i != me)
			{
				if (d < min_d)
					min_d = d;
			}
			else
			{
				if (d < 1024) //to near to me!!!
					return LEVEL_WIDTH*LEVEL_WIDTH;
			}
			hare = hare->next;
		}
		while (hare != player[i]->firstHare);
	}
	return min_d;
}


void add_ms_to_data(int ms)
{
	if (ms % 2 == 0)
	{
		unsigned char send_byte =
			((input_states[ 0] & 1) << 0) |
			((input_states[ 1] & 1) << 1) |
			((input_states[ 2] & 1) << 2) |
			((input_states[ 3] & 1) << 3) |
			((input_states[ 4] & 1) << 4) |
			((input_states[ 5] & 1) << 5) |
			((input_states[ 6] & 1) << 6) |
			((input_states[ 7] & 1) << 7) ;
		send_data[ms*3/2] = send_byte;
		send_byte =
			((input_states[ 8] & 1) << 0) |
			((input_states[ 9] & 1) << 1) |
			((input_states[10] & 1) << 2) |
			((input_states[11] & 1) << 3) ;
		send_data[ms*3/2+1] |= send_byte;
	}
	else
	{
		unsigned char send_byte =
			((input_states[ 0] & 1) << 4) |
			((input_states[ 1] & 1) << 5) |
			((input_states[ 2] & 1) << 6) |
			((input_states[ 3] & 1) << 7) ;
		send_data[ms*3/2] |= send_byte;
		send_byte =
			((input_states[ 4] & 1) << 0) |
			((input_states[ 5] & 1) << 1) |
			((input_states[ 6] & 1) << 2) |
			((input_states[ 7] & 1) << 3) |
			((input_states[ 8] & 1) << 4) |
			((input_states[ 9] & 1) << 5) |
			((input_states[10] & 1) << 6) |
			((input_states[11] & 1) << 7) ;
		send_data[ms*3/2+1] = send_byte;
	}
}

void get_ms_from_data(int ms)
{
	if (ms % 2 == 0)
	{
		unsigned char send_byte = send_data[ms*3/2];
		input_states[ 0] = (send_byte >> 0) & 1;
		input_states[ 1] = (send_byte >> 1) & 1;
		input_states[ 2] = (send_byte >> 2) & 1;
		input_states[ 3] = (send_byte >> 3) & 1;
		input_states[ 4] = (send_byte >> 4) & 1;
		input_states[ 5] = (send_byte >> 5) & 1;
		input_states[ 6] = (send_byte >> 6) & 1;
		input_states[ 7] = (send_byte >> 7) & 1;
		send_byte = send_data[ms*3/2+1];
		input_states[ 8] = (send_byte >> 0) & 1;
		input_states[ 9] = (send_byte >> 1) & 1;
		input_states[10] = (send_byte >> 2) & 1;
		input_states[11] = (send_byte >> 3) & 1;
	}
	else
	{
		unsigned char send_byte = send_data[ms*3/2];
		input_states[ 0] = (send_byte >> 4) & 1;
		input_states[ 1] = (send_byte >> 5) & 1;
		input_states[ 2] = (send_byte >> 6) & 1;
		input_states[ 3] = (send_byte >> 7) & 1;
		send_byte = send_data[ms*3/2+1];
		input_states[ 4] = (send_byte >> 0) & 1;
		input_states[ 5] = (send_byte >> 1) & 1;
		input_states[ 6] = (send_byte >> 2) & 1;
		input_states[ 7] = (send_byte >> 3) & 1;
		input_states[ 8] = (send_byte >> 4) & 1;
		input_states[ 9] = (send_byte >> 5) & 1;
		input_states[10] = (send_byte >> 6) & 1;
		input_states[11] = (send_byte >> 7) & 1;
	}
}

int last_heartbeat_diff;

void set_input()
{
	if (player[active_player]->computer)
		memset(input_states,0,sizeof(int)*12);
	else
	if (player[active_player]->local)
	{
		if (spMapGetByID(MAP_VIEW) == 0 && chatWindow == NULL)
		{
			if (spGetInput()->axis[0] < 0)
			{
				input_states[INPUT_AXIS_0_LEFT] = 1;
				input_states[INPUT_AXIS_0_RIGHT] = 0;
			}
			else
			if (spGetInput()->axis[0] > 0)
			{
				input_states[INPUT_AXIS_0_LEFT] = 0;
				input_states[INPUT_AXIS_0_RIGHT] = 1;
			}
			else
			{
				input_states[INPUT_AXIS_0_LEFT] = 0;
				input_states[INPUT_AXIS_0_RIGHT] = 0;
			}		
			if (gop_direction_flip() && !gop_rotation() && player[active_player]->activeHare->rotation > SP_PI/2 && player[active_player]->activeHare->rotation < SP_PI*3/2)
			{
				int temp = input_states[INPUT_AXIS_0_LEFT];
				input_states[INPUT_AXIS_0_LEFT] = input_states[INPUT_AXIS_0_RIGHT];
				input_states[INPUT_AXIS_0_RIGHT] = temp;
			}
			if (spGetInput()->axis[1] < 0)
			{
				input_states[INPUT_AXIS_1_LEFT] = 1;
				input_states[INPUT_AXIS_1_RIGHT] = 0;
			}
			else
			if (spGetInput()->axis[1] > 0)
			{
				input_states[INPUT_AXIS_1_LEFT] = 0;
				input_states[INPUT_AXIS_1_RIGHT] = 1;
			}
			else
			{
				input_states[INPUT_AXIS_1_LEFT] = 0;
				input_states[INPUT_AXIS_1_RIGHT] = 0;
			}
			if (spMapGetByID(MAP_POWER_DN) != button_states[MAP_POWER_DN])
				input_states[INPUT_BUTTON_L] = spMapGetByID(MAP_POWER_DN);
			if (spMapGetByID(MAP_POWER_UP) != button_states[MAP_POWER_UP])
				input_states[INPUT_BUTTON_R] = spMapGetByID(MAP_POWER_UP);
			if (spMapGetByID(MAP_JUMP) != button_states[MAP_JUMP])
				input_states[INPUT_BUTTON_OK] = spMapGetByID(MAP_JUMP);
			if (spMapGetByID(MAP_WEAPON) != button_states[MAP_WEAPON])
				input_states[INPUT_BUTTON_3] = spMapGetByID(MAP_WEAPON);
			if (spMapGetByID(MAP_SHOOT) != button_states[MAP_SHOOT])
				input_states[INPUT_BUTTON_CANCEL] = spMapGetByID(MAP_SHOOT);
			if (spMapGetByID(MAP_VIEW) != button_states[MAP_VIEW])
				input_states[INPUT_BUTTON_4] = spMapGetByID(MAP_VIEW);

			if (spMapGetByID(MAP_CHAT) != button_states[MAP_CHAT])
				input_states[INPUT_BUTTON_START] = spMapGetByID(MAP_CHAT);
			if (spMapGetByID(MAP_MENU) != button_states[MAP_MENU])
				input_states[INPUT_BUTTON_SELECT] = spMapGetByID(MAP_MENU);
		}
		else
			memset(input_states,0,sizeof(int)*12);
		//I need to save the button states to compare, to be able to set input_states[…] to 0!
		int i;
		for (i = 0; i < 8; i++)
			button_states[i] = spMapGetByID(i);
		if (!hase_game->local)
		{
			add_ms_to_data(player[active_player]->time % 1000);
			if (player[active_player]->time % 1000 == 999)
			{
				printf("Pushing Second %i of player %s...\n",player[active_player]->time/1000,player[active_player]->name);
				push_game_thread(player[active_player],player[active_player]->time/1000,send_data);
				memset(send_data,0,sizeof(char)*1536);
			}
		}
		player[active_player]->time++;
	}
	else //online
	{
		if (player[active_player]->time % 1000 == 0)
		{
			printf("Pulling Second %i of player %s...\n",player[active_player]->time/1000,player[active_player]->name);
			game_pause = pull_game_thread(player[active_player],player[active_player]->time/1000,send_data)*1000;
			printf("Done with status: %i\n",game_pause);
			if (game_pause)
			{
				char buffer[256];
				if (last_heartbeat_diff < 0)
					sprintf(buffer,"Waiting for turn data\nfrom player %s...\n(Player status unknown)",player[active_player]->name);
				else
				if (last_heartbeat_diff < 15)
					sprintf(buffer,"Waiting for turn data\nfrom player %s...\n(Player is alive, be patient)",player[active_player]->name);
				else
				if (last_heartbeat_diff < 90)
					sprintf(buffer,"Waiting for turn data\nfrom player %s...\n(Last vital sign %i seconds ago, kick at 90)",player[active_player]->name,last_heartbeat_diff);
				else
					sprintf(buffer,"Waiting for turn data\nfrom player %s...",player[active_player]->name);				
				set_message(font,buffer);
			}	
		}
		if (game_pause == 0)
		{
			get_ms_from_data(player[active_player]->time % 1000);
			player[active_player]->time++;
		}
	}
}



int quit_feedback( pWindow window, pWindowElement elem, int action )
{
	sprintf(elem->text,"Do you really want to quit?");
	return 0;
}

int calc(Uint32 steps)
{
	steps *= speed;
	#ifdef PROFILE
		int start_time = SDL_GetTicks();
	#endif
	try_to_join();
	if (get_channel())
	{
		spNetIRCMessagePointer showing = NULL;
		if (before_showing == NULL && get_channel()->first_message)
			showing = get_channel()->first_message;
		else
		if (before_showing)
			showing = before_showing->next;
		time_t now = time(NULL) - 60;
		//Adding new chat lines
		while (showing)
		{
			char buffer[4096];
			spFontPointer used_font = NULL;
			char* message = showing->message;
			if (hase_game->status == -1 || //replay
				(message = ingame_message(showing->message,hase_game->name)))
			{
				used_font = font;
				sprintf(buffer,"%s: %s",showing->user,message);
			}
			else
			if (gop_global_chat())
			{
				used_font = font_dark;
				sprintf(buffer,"%s: %s",showing->user,showing->message);
			}
			if (used_font)
			{
				char* found = buffer;
				while (spFontWidth(found,used_font) > screen->w)
				{
					char* end_space = found;
					char* last_space = NULL;
					end_space++;
					int was_null = 0;
					while (end_space[0])
					{
						while (end_space[0] && end_space[0] != ' ')
							end_space++;
						if (end_space[0] == 0)
							was_null = 1;
						end_space[0] = 0;
						if (spFontWidth(found,used_font) > screen->w)
						{
							if (!was_null)
								end_space[0] = ' ';
							break;
						}
						if (!was_null)
							end_space[0] = ' ';
						last_space = end_space;
						if (!was_null)
							end_space++;
					}
					if (last_space != NULL)
						end_space = last_space;
					end_space[0] = 0;
					pChatLine line = (pChatLine)malloc(sizeof(tChatLine));
					strncpy(line->line,found,1023);
					line->line[1023] = 0;
					line->font = used_font;
					line->t = showing->time_stamp;
					line->next = NULL;
					if (lastChatLine)
						lastChatLine->next = line;
					else
						firstChatLine = line;
					lastChatLine = line;
					chatLineCount++;
					found = end_space;
					if (last_space)
						found++;
				}
				pChatLine line = (pChatLine)malloc(sizeof(tChatLine));
				strncpy(line->line,found,1023);
				line->line[1023] = 0;
				line->font = used_font;
				line->t = showing->time_stamp;
				line->next = NULL;
				if (lastChatLine)
					lastChatLine->next = line;
				else
					firstChatLine = line;
				lastChatLine = line;
				chatLineCount++;
			}
			before_showing = showing;
			showing = showing->next;
		}
		//Removing old chat lines
		while (firstChatLine && (firstChatLine->t < now || chatLineCount > spFixedToInt(spGetSizeFactor() * 5)))
		{
			pChatLine next = firstChatLine->next;
			free(firstChatLine);
			chatLineCount--;
			if (lastChatLine == firstChatLine)
				lastChatLine = NULL;
			firstChatLine = next;
		}
	}
	if (spMapGetByID(MAP_MENU))
	{
		spMapSetByID(MAP_MENU,0);
		spSoundPause(1,-1);
		spMapSetMapSet(0);
		if (options_window(font,hase_resize,1))
		{
			pWindow window = create_window(quit_feedback,font,"Sure?");
			add_window_element(window,-1,0);
			int res = modal_window(window,hase_resize);
			delete_window(window);
			if (res == 1)
				return 1;
		}
		spMapSetMapSet(1);
		spSoundPause(0,-1);
	}
	
	if (chatWindow)
	{
		spMapSetMapSet(0);
		int result = window_calc(steps);
		if (result == 1)
		{
			if (chatMessage[0])
				send_chat(hase_game,chatMessage);
			chatMessage[0] = 0;
		}
		if (result)
		{
			delete_window(chatWindow);
			chatWindow = NULL;
		}
		spMapSetMapSet(1);
	}
	
	if (spMapGetByID(MAP_CHAT) && get_channel())
	{
		spMapSetByID(MAP_CHAT,0);
		spMapSetMapSet(0);
		chatWindow = create_text_box(font,hase_resize,"Enter Message:",chatMessage,256,0,NULL,1);
		chatWindow->do_flip = 0;
		set_recent_window(chatWindow);
		spMapSetMapSet(1);
	}
	if (spMapGetByID(MAP_VIEW))
	{
		if (spMapGetByID(MAP_JUMP))
		{
			spMapSetByID(MAP_JUMP,0);
			help = 1-help;
		}
		if (spMapGetByID(MAP_WEAPON))
		{
			spMapSetByID(MAP_WEAPON,0);
			speed *= 2;
		}
		if (spMapGetByID(MAP_SHOOT))
		{
			spMapSetByID(MAP_SHOOT,0);
			speed /= 2;
			if (speed <= 1)
				speed = 1;
		}
		if (spMapGetByID(MAP_POWER_DN))
			zoom_d = -1;
		if (spMapGetByID(MAP_POWER_UP))
			zoom_d =  1;
		if (spGetInput()->axis[0] < 0)
		{
			posX -= spDiv(spCos(-rotation),zoom*3/4)*steps;
			posY -= spDiv(spSin(-rotation),zoom*3/4)*steps;
		}
		if (spGetInput()->axis[0] > 0)
		{
			posX -= spDiv(spCos(-rotation+SP_PI),zoom*3/4)*steps;
			posY -= spDiv(spSin(-rotation+SP_PI),zoom*3/4)*steps;
		}
		if (spGetInput()->axis[1] < 0)
		{
			posX -= spDiv(spCos(-rotation+SP_PI/2),zoom*3/4)*steps;
			posY -= spDiv(spSin(-rotation+SP_PI/2),zoom*3/4)*steps;
		}
		if (spGetInput()->axis[1] > 0)
		{
			posX -= spDiv(spCos(-rotation+3*SP_PI/2),zoom*3/4)*steps;
			posY -= spDiv(spSin(-rotation+3*SP_PI/2),zoom*3/4)*steps;
		}
	}
	int i;
	update_player_sprite(steps);
	int result = 0;
	if (game_pause)
	{
		spSoundPause(1,-1);
		spSleep(100000);
	}
	else
		spSoundPause(0,-1);
	spUpdateSprite(spActiveSprite(targeting),steps);
	spParticleUpdate(&particles,steps);
	for (i = 0; i < steps; i++)
	{
		if (zoom_d == -1)
		{
			zoomAdjust -= 32;
			if (zoomAdjust < minZoom)
				zoomAdjust = minZoom;
			zoom = spMul(zoomAdjust,zoomAdjust);
			if ((zoomAdjust & 16383) == 0 && gop_zoom())
				zoom_d = 0;
			if (gop_zoom() == 0 && spMapGetByID(MAP_POWER_DN) == 0)
				zoom_d = 0;
		}
		else
		if (zoom_d == 1)
		{
			zoomAdjust += 32;
			if (zoomAdjust > maxZoom)
				zoomAdjust = maxZoom;
			zoom = spMul(zoomAdjust,zoomAdjust);
			if ((zoomAdjust & 16383) == 0 && gop_zoom())
				zoom_d = 0;
			if (gop_zoom() == 0 && spMapGetByID(MAP_POWER_UP) == 0)
				zoom_d = 0;
		}
		//Camera
		if (player[active_player]->activeHare)
		{
			int destX,destY;
			if (dropItem && dropItem->seen)
			{
				destX = dropItem->x;
				destY = dropItem->y;
			}
			else
			{
				destX = player[active_player]->activeHare->x;
				destY = player[active_player]->activeHare->y;
				if (firstBullet)
				{
					pBullet momBullet = firstBullet;
					int c = 1;
					while (momBullet)
					{
						destX += momBullet->x*2;
						destY += momBullet->y*2;
						momBullet = momBullet->next;
						c+=2;
					}
					destX /= c;
					destY /= c;
				}
			}
			posX = ((Sint64)posX*(Sint64)255+(Sint64)destX) >> 8;
			posY = ((Sint64)posY*(Sint64)255+(Sint64)destY) >> 8;
		}
		if (player[active_player]->activeHare)
		{
			Sint32 goal = -player[active_player]->activeHare->cam_rotation;
			if (goal < -SP_PI*3/2 && rotation > -SP_PI/2)
				rotation -= 2*SP_PI;
			if (goal > -SP_PI/2 && rotation < -SP_PI*3/2)
				rotation += 2*SP_PI;
			rotation = rotation*127/128+goal/128;
		}
		if (game_pause > 0)
			game_pause--;
		if (game_pause)
			continue;
		set_input();
		if (game_pause)
			continue;
		items_calc();
		update_player();
		int res = do_physics();
		if (res == 1)
			result = 2;
		if (res)
		{
			i = steps;
			continue;
		}
		if (bullet_alpha() > 0)
			continue;
		if (check_next_player())
			return 3;
		if (player[active_player]->weapon_points)
			countdown--;
		if (countdown == 5000 && speed == 1)
			channel_end = spSoundPlay(snd_end,-1,0,0,-1);
		if (countdown < 0)
			next_player();
		Sint32 old_power = player[active_player]->activeHare->w_power;
		if (wp_choose == 0)
		{
			int w_nr = weapon_pos[player[active_player]->activeHare->wp_y][player[active_player]->activeHare->wp_x];
			if (player[active_player]->computer && player[active_player]->activeHare)
			{		
				//AI
				if (player[active_player]->weapon_points)
				{
					if (player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
					{
						if (ai_shoot_tries == 0)
						{
							jump((spRand()%4==0)?1:0);
							if (spRand()%4 == 0)
								ai_shoot_tries = 1;
						}
						else
						{
							if (last_ai_try < AI_TRIES_EVERY_MS)
								last_ai_try++;
							else
							{
								last_ai_try = 0;
								ai_shoot_tries++;
								if (ai_shoot_tries < AI_MAX_TRIES)
								{
									//Lets first try...
									int x = player[active_player]->activeHare->x;
									int y = player[active_player]->activeHare->y;
									int w_d = spRand()%(2*SP_PI);
									int w_p = spRand()%SP_ONE;
									lastPoint(&x,&y,player[active_player]->activeHare->rotation+w_d+SP_PI,w_p/2);
									int d = min_d_not_me(x,y,active_player);
									if (d < lastAIDistance)
									{
										lastAIDistance = d;
										player[active_player]->activeHare->w_direction = w_d;
										player[active_player]->activeHare->w_power = w_p;
									}
								}
								else
								{
									//Shoot!
									if (player[active_player]->weapon_points > 0)
									{
										player[active_player]->weapon_points-=weapon_cost[WP_BIG_BAZOOKA];
										shootBullet(player[active_player]->activeHare->x,player[active_player]->activeHare->y,player[active_player]->activeHare->w_direction+player[active_player]->activeHare->rotation+SP_PI,player[active_player]->activeHare->w_power/2,player[active_player]->activeHare->direction?1:-1,player[active_player],weapon_surface[w_nr],WP_BIG_BAZOOKA,1);
									}
									break;
								}
							}
						}
					}
				}
				else
				{
					//RUNNING!
					if (player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
						jump((spRand()%4==0)?1:0);
				}
			}
			else
			if (player[active_player]->activeHare)
			{
				//not AI
				if (input_states[INPUT_BUTTON_OK] && player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
				{
					jump(1);
				}
				if (input_states[INPUT_AXIS_0_LEFT])
				{
					if (player[active_player]->activeHare->direction == 1)
					{
						player[active_player]->activeHare->direction = 0;
						player[active_player]->activeHare->w_direction = SP_PI-player[active_player]->activeHare->w_direction;
						direction_hold = 0;
					}
					direction_hold++;
					if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
						jump(0);
				}
				else
				if (input_states[INPUT_AXIS_0_RIGHT])
				{
					if (player[active_player]->activeHare->direction == 0)
					{
						player[active_player]->activeHare->direction = 1;
						player[active_player]->activeHare->w_direction = SP_PI-player[active_player]->activeHare->w_direction;
						direction_hold = 0;
					}
					direction_hold++;
					if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
						jump(0);
				}
				else
					direction_hold = 0;
				
				if (input_states[INPUT_AXIS_1_LEFT])
				{
					direction_pressed += SP_ONE/20;
					if (direction_pressed >= 128*SP_ONE)
						direction_pressed = 128*SP_ONE;
					if (player[active_player]->activeHare->direction == 0)
						player[active_player]->activeHare->w_direction += direction_pressed >> SP_ACCURACY;
					else
						player[active_player]->activeHare->w_direction -= direction_pressed >> SP_ACCURACY;
				}
				else
				if (input_states[INPUT_AXIS_1_RIGHT])
				{
					direction_pressed += SP_ONE/20;
					if (direction_pressed >= 128*SP_ONE)
						direction_pressed = 128*SP_ONE;
					if (player[active_player]->activeHare->direction == 0)
						player[active_player]->activeHare->w_direction -= direction_pressed >> SP_ACCURACY;
					else
						player[active_player]->activeHare->w_direction += direction_pressed >> SP_ACCURACY;
				}
				else
					direction_pressed = 0;

				if (input_states[INPUT_BUTTON_R])
				{
					power_pressed += SP_ONE/100;
					player[active_player]->activeHare->w_power += power_pressed >> SP_ACCURACY;
					if (player[active_player]->activeHare->w_power >= SP_ONE)
						player[active_player]->activeHare->w_power = SP_ONE;
				}
				else
				if (input_states[INPUT_BUTTON_L])
				{
					power_pressed += SP_ONE/100;
					player[active_player]->activeHare->w_power -= power_pressed >> SP_ACCURACY;
					if (player[active_player]->activeHare->w_power < 0)
						player[active_player]->activeHare->w_power = 0;
				}
				else
					power_pressed = 0;
				if (input_states[INPUT_BUTTON_CANCEL] && player[active_player]->activeHare)
				{
					//Shoot!
					if (player[active_player]->weapon_points - weapon_cost[w_nr] >= 0)
					{
						input_states[INPUT_BUTTON_CANCEL] = 0;
						player[active_player]->weapon_points-=weapon_cost[w_nr];
						if (weapon_shoot[w_nr])
							shootBullet(player[active_player]->activeHare->x,player[active_player]->activeHare->y,player[active_player]->activeHare->w_direction+player[active_player]->activeHare->rotation+SP_PI,player[active_player]->activeHare->w_power/2,player[active_player]->activeHare->direction?1:-1,player[active_player],weapon_surface[w_nr],w_nr,1);
						else
						switch (w_nr)
						{
							case WP_BUILD_SML:case WP_BUILD_MID:case WP_BUILD_BIG:
								{
									int d = 12+weapon_explosion[w_nr]+(player[active_player]->activeHare->w_power*(12+weapon_explosion[w_nr]) >> SP_ACCURACY);
									int r = weapon_explosion[w_nr];
									int ox = player[active_player]->activeHare->x-d*-spMul(spSin(player[active_player]->activeHare->rotation+player[active_player]->activeHare->w_direction-SP_PI/2),player[active_player]->activeHare->w_power+SP_ONE*2/3);
									int oy = player[active_player]->activeHare->y-d* spMul(spCos(player[active_player]->activeHare->rotation+player[active_player]->activeHare->w_direction-SP_PI/2),player[active_player]->activeHare->w_power+SP_ONE*2/3);
									if (circle_is_empty(ox>>SP_ACCURACY,oy>>SP_ACCURACY,r/2+4,NULL,-1))
										negative_impact(ox>>SP_ACCURACY,oy>>SP_ACCURACY,r/2);
									else
										player[active_player]->weapon_points+=weapon_cost[w_nr];
								}
								break;
							case WP_SUPER_JUMP:
								if (player[active_player]->activeHare->bums && player[active_player]->activeHare->hops <= 0)
									jump(2); //jump is possible
								else
									player[active_player]->weapon_points+=weapon_cost[w_nr]; //payback						
								break;
							case WP_KAIO_KEN:
								player[active_player]->next_round_extra++;
								break;
							case WP_PREV_HARE:
								player[active_player]->activeHare = player[active_player]->activeHare->before;
								break;
							case WP_NEXT_HARE:
								player[active_player]->activeHare = player[active_player]->activeHare->next;
								break;
						}
					}
				}
			}
			if (player[active_player]->activeHare->w_power != old_power)
				update_targeting();
		}
		if (firstBullet == NULL && player[active_player]->weapon_points == 0)
		{
			if (extra_time == 0)
				extra_time = 5000;
			else
			if (extra_time == 1)
				next_player();
			else
				extra_time--;
		}
		if (firstBullet || player[active_player]->weapon_points == 0)
			if (channel_end >= 0)
			{
				spSoundStop(channel_end,0);
				channel_end = -1;
			}

		
		if (wp_choose)
		{
			if (input_states[INPUT_AXIS_0_LEFT] && player[active_player]->activeHare)
			{
				if (wp_choose == 1)
				{
					wp_choose = 300;
					player[active_player]->activeHare->wp_x = (player[active_player]->activeHare->wp_x + WEAPON_X - 1) % WEAPON_X;
				}
			}
			else
			if (input_states[INPUT_AXIS_0_RIGHT] && player[active_player]->activeHare)
			{
				if (wp_choose == 1)
				{
					wp_choose = 300;
					player[active_player]->activeHare->wp_x = (player[active_player]->activeHare->wp_x + 1) % WEAPON_X;
				}
			}
			else
			if (input_states[INPUT_AXIS_1_LEFT] && player[active_player]->activeHare)
			{
				if (wp_choose == 1)
				{
					wp_choose = 300;
					player[active_player]->activeHare->wp_y = (player[active_player]->activeHare->wp_y + WEAPON_Y - 1) % WEAPON_Y;
				}
			}
			else
			if (input_states[INPUT_AXIS_1_RIGHT] && player[active_player]->activeHare)
			{
				if (wp_choose == 1)
				{
					wp_choose = 300;
					player[active_player]->activeHare->wp_y = (player[active_player]->activeHare->wp_y + 1) % WEAPON_Y;
				}
			}
			else
				wp_choose = 1;
			if (wp_choose > 1)
				wp_choose--;
			if (input_states[INPUT_BUTTON_3])
			{
				input_states[INPUT_BUTTON_3] = 0;
				wp_choose = 0;
			}
			if (input_states[INPUT_BUTTON_OK])
			{
				input_states[INPUT_BUTTON_OK] = 0;
				wp_choose = 0;
			}
			continue;
		}
		else
		if (input_states[INPUT_BUTTON_3])
		{
			input_states[INPUT_BUTTON_3] = 0;
			wp_choose = 1;
			continue;
		}
	}
	//Add snow
	if (winter && gop_particles() < 4)
	{
		int c = (steps << 2) >> gop_particles();
		spParticleBunchPointer p = spParticleCreate(c,hare_explosion_feedback,&particles);
		int i;
		for (i = 0; i < c; i++)
		{
			p->particle[i].x = rand() % LEVEL_WIDTH << SP_ACCURACY;
			p->particle[i].y = rand() % LEVEL_HEIGHT << SP_ACCURACY;
			p->particle[i].z = 0;
			p->particle[i].dx = 0;
			p->particle[i].dy = 0;
			p->particle[i].dz = 0;
			p->particle[i].data.color = 65535;
			/*if (gravitation_force(p->particle[i].x >> SP_ACCURACY,p->particle[i].y >> SP_ACCURACY) == 0 ||
				gravitation_force(p->particle[i].x >> SP_ACCURACY,p->particle[i].y >> SP_ACCURACY) > SP_ONE/2)
				p->particle[i].status = -1;
			else*/
				p->particle[i].status = 0;
		}		
	}
	#ifdef PROFILE
		calc_time = SDL_GetTicks() - start_time;
	#endif
	return result;
}

void update_map()
{
	Uint16 c = get_level_color();
	Uint16 b = get_border_color();
	SDL_LockSurface( map_surface );
	SDL_LockSurface( level_original );
	int map_s = map_surface->pitch/map_surface->format->BytesPerPixel;
	int lvl_s = level_original->pitch/level_original->format->BytesPerPixel;
	Uint16* map = ( Uint16* )( map_surface->pixels );
	Uint16* lvl = ( Uint16* )( level_original->pixels );
	int x,y;
	for ( x = 0; x < map_w; x++)
		for ( y = 0; y < map_h; y++)
		{
			int lx = x*LEVEL_WIDTH/map_w;
			int ly = y*LEVEL_HEIGHT/map_h;
			if (x == 0 ||
				x == map_w-1 ||
				y == 0 ||
				y == map_h-1)
			map[x+y*map_s] = b;
			else
			if (lvl[lx+ly*lvl_s] != SP_ALPHA_COLOR)
				map[x+y*map_s] = c;
			else
				map[x+y*map_s] = SP_ALPHA_COLOR;
		}
	for ( x = 1; x < map_w-1; x++)
		for ( y = 1; y < map_h-1; y++)
		{
			if (map[x+y*map_s] != SP_ALPHA_COLOR &&
				(map[x+1+y*map_s] == SP_ALPHA_COLOR ||
				 map[x-1+y*map_s] == SP_ALPHA_COLOR ||
				 map[x+map_s+y*map_s] == SP_ALPHA_COLOR ||
				 map[x-map_s+y*map_s] == SP_ALPHA_COLOR))
				map[x+y*map_s] = b;
		}
	SDL_UnlockSurface( map_surface );
	SDL_UnlockSurface( level_original );
}

int hase(void ( *resize )( Uint16 w, Uint16 h ),pGame game,pPlayer me_list)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (tm.tm_mon == 11 &&
		tm.tm_mday >= 24 &&
		tm.tm_mday <= 26)
		winter = 1;
	else
		winter = 0;
	screen = spGetWindowSurface();
	chatWindow = NULL;
	chatMessage[0] = 0;
	before_showing = NULL;
	hase_resize = resize;
	hase_game = game;
	get_game(game,&hase_player_list);
	pPlayer p = hase_player_list;
	while (p)
	{
		p->local = 0;
		pPlayer q = me_list;
		while (q)
		{
			if (p->id == q->id)
			{
				p->local = 1;
				p->pw = q->pw;
				break;
			}	
			q = q->next;
		}
		p = p->next;
	}
	//Getting a deterministic seed
	Uint32 f[4] = {123,123,123,123};
	int k;
	for (k = 0; k < 4 && game->name[k]; k++)
		f[k] = game->name[k];
	Uint32 seed = f[0]+f[1]*256+f[2]*65536+f[3]*16777216;
	spSetRand(seed);
	loadInformation("Loading images...");
	targeting = spLoadSpriteCollection("./data/targeting.ssc",NULL);
	arrow = spLoadSurface("./data/gravity.png");
	tomato = spLoadSurfaceZoom("./data/power.png",spGetSizeFactor()/4);
	load_weapons();
	gravity_surface = spCreateSurface( GRAVITY_DENSITY << (GRAVITY_RESOLUTION+1), GRAVITY_DENSITY << (GRAVITY_RESOLUTION+1));
	loadInformation("Creating level...");
	level_original = create_level(game->level_string,0,0,65535);
	texturize_level(level_original,game->level_string);
	loadInformation("Created Arrow image...");
	fill_gravity_surface();
	level = spCreateSurface(LEVEL_WIDTH,LEVEL_HEIGHT);
	loadInformation("Created new surface...");
	level_pixel = (Uint16*)level_original->pixels;
	realloc_gravity();
	init_gravity();
	init_player(hase_player_list,game->player_count,game->hares_per_player);
	items_init(game);
	map_w = 64 * spGetSizeFactor() >> SP_ACCURACY;
	map_h = 64 * spGetSizeFactor() >> SP_ACCURACY;
	map_size = spGetSizeFactor() >> SP_ACCURACY;
	map_surface = spCreateSurface( map_w, map_h );
	update_map();
	zoomAdjust = spSqrt(spGetSizeFactor());
	minZoom = spSqrt(spGetSizeFactor()/4)/16384*16384;
	maxZoom = spSqrt(spGetSizeFactor()*4)/16384*16384;
	zoom = spMul(zoomAdjust,zoomAdjust);
	zoom_d = 0;
	speed = 1;
	countdown = hase_game->seconds_per_turn*1000;
	alive_count = player_count;
	memset(input_states,0,sizeof(int)*12);
	memset(button_states,0,sizeof(char)*8);
	memset(send_data,0,1536*sizeof(char));
	game_pause = 0;
	wp_choose = 0;
	snd_explosion = spSoundLoad("./sounds/explosion.wav");
	snd_beep = spSoundLoad("./sounds/beep.wav");
	snd_end = spSoundLoad("./sounds/end_beep.wav");
	channel_end = -1;
	snd_high = spSoundLoad("./sounds/high_jump.wav");
	snd_low = spSoundLoad("./sounds/short_jump.wav");
	snd_shoot = spSoundLoad("./sounds/plop.wav");
	snd_turn = spSoundLoad("./sounds/your_turn.wav");
	
	spMapSetMapSet(1);
	
	if (player[active_player]->local)
		spSoundPlay(snd_turn,-1,0,0,-1);

	int result = spLoop(draw,calc,10,resize,NULL);
	
	spMapSetMapSet(0);
	
	spSoundStop(-1,0);
	spSoundDelete(snd_explosion);
	spSoundDelete(snd_beep);
	spSoundDelete(snd_end);
	spSoundDelete(snd_high);
	spSoundDelete(snd_low);
	spSoundDelete(snd_shoot);
	spSoundDelete(snd_turn);

	stop_thread(result < 2);
	if (result == 3)
		message_box(font,hase_resize,"You got disconnected!");
	if (result == 2)
	{
		int i;
		for (i = 0; i < player_count; i ++)
			if (player[i]->firstHare)
				break;
		if (i < player_count)
		{
			char buffer[256];
			sprintf(buffer,"%s won!\n",player[i]->name);
			message_box(font,hase_resize,buffer);
		}
		else
			message_box(font,hase_resize,"Nobody won, but why?");
	}
	deleteAllBullets();
	free_gravity();
	int i;
	for (i = 0; i < player_count; i++)
	{
		pHare hare = player[i]->firstHare;
		while (hare)
		{
			spDeleteSpriteCollection(hare->hase,0);
			pHare next = hare->next;
			free(hare);
			hare = next;
			if (hare == player[i]->firstHare)
				break;
		}
		deleteAllTraces(player[i]);
	}
	spDeleteSurface(map_surface);
	spDeleteSpriteCollection(targeting,0);
	spDeleteSurface(arrow);
	spDeleteSurface(level);
	spDeleteSurface(level_original);
	spDeleteSurface(gravity_surface);
	spDeleteSurface(tomato);
	delete_weapons();
	spParticleDelete(&particles);
	spResetButtonsState();
	items_quit();
	if (chatWindow)
		delete_window(chatWindow);
	while (firstChatLine)
	{
		pChatLine next = firstChatLine->next;
		free(firstChatLine);
		firstChatLine = next;
	}
	chatLineCount = 0;
	lastChatLine = NULL;
	return result;
}
