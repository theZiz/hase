#include <sparrow3d.h>
#include <stdlib.h>
#include <string.h>

#include "hase.h"

pGame hase_game;
pPlayer hase_player_list;

#define LEVEL_WIDTH 1536
#define LEVEL_HEIGHT 1536

SDL_Surface* screen;
spFontPointer font;
SDL_Surface* level;
SDL_Surface* level_original;
Uint16* level_pixel;
SDL_Surface* arrow;
SDL_Surface* weapon;
SDL_Surface* bullet;
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
int weapon_points = 0;

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
char button_states[SP_INPUT_BUTTON_COUNT];
unsigned char send_data[1536];

void ( *hase_resize )( Uint16 w, Uint16 h );

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

#include "level.h"

void draw(void)
{
	char buffer[256];
	spClearTarget(0);
	spSetFixedOrign(posX >> SP_ACCURACY,posY >> SP_ACCURACY);
	spSetVerticalOrigin(SP_FIXED);
	spSetHorizontalOrigin(SP_FIXED);
	
	//Level
	spRotozoomSurface(screen->w/2,screen->h/2,0,level,zoom,zoom,rotation);
	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);

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
			spSetSpriteRotation(sprite,+rotation+hare->rotation);
			Sint32 ox = spMul(hare->x-posX,zoom);
			Sint32 oy = spMul(hare->y-posY,zoom);
			Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
			Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
			if (j == active_player)
			{
				//Weapon
				spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,weapon,zoom/2,zoom/2,hare->w_direction+rotation+hare->rotation);
				//Arrow
				if (hare->w_power)
				{
					Sint32 ox = spMul(hare->x-posX-14*-spMul(spSin(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32 oy = spMul(hare->y-posY-14* spMul(spCos(hare->rotation+hare->w_direction-SP_PI/2),hare->w_power+SP_ONE*2/3),zoom);
					Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
					Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
					spSetBlending( SP_ONE*2/3 );
					spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,arrow,spMul(zoom,spGetSizeFactor())/16,spMul(hare->w_power,spMul(zoom,spGetSizeFactor()))/4,hare->w_direction+rotation+hare->rotation-SP_PI/2);
					spSetBlending( SP_ONE );
				}
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
			spFontDrawMiddle( screen->w/2+x,screen->h/2+y-font->maxheight,0,buffer, font );
			if (j == active_player && player[j]->computer && ai_shoot_tries>1)
			{
				sprintf(buffer,"Aiming (%2i%%)",ai_shoot_tries*100/AI_MAX_TRIES);
				spFontDrawMiddle( screen->w/2+x,screen->h/2+y-2*font->maxheight,0,buffer, font );
			}
			spSetBlending( SP_ONE );
			hare = hare->next;
		}
		while (hare != player[j]->firstHare);
	}
	//Bullets
	drawBullets();
		
	//Trace
	drawTrace(player[active_player]);
	
	//Error message
	if (game_pause)
		draw_message();
	
	//HID
	int y = screen->h - font->maxheight - 1;
	spSetBlending( SP_ONE*2/3 );
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->firstHare == NULL)
			continue;
		sprintf(buffer,"%s ",player[j]->name);
		spFontDrawRight( screen->w/2, y, 0, buffer, font );
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
		int w = health*screen->w/(hase_game->hares_per_player*MAX_HEALTH*4);
		spRectangle(screen->w/2+w/2,y+font->maxheight/2,0,w,font->maxheight*3/4,spSpriteAverageColor(hare->hase->active));
		sprintf(buffer,"%i/%i",count,hase_game->hares_per_player);
		spFontDraw( screen->w/2+w+2, y, 0, buffer, font );
		y -= font->maxheight;
	}
	spSetBlending( SP_ONE );
	
	
	sprintf(buffer,"Weapon points: %i/3",weapon_points);
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	sprintf(buffer,"Power: %i %%",player[active_player]->activeHare->w_power*100/SP_ONE);
	spFontDraw( 2, 2, 0, buffer, font );	
	if (weapon_points)
		sprintf(buffer,"%i seconds left",countdown / 1000);
	else
	if (extra_time)
		sprintf(buffer,"%i seconds left to escape",extra_time / 1000);
	else
		sprintf(buffer,"Free time until bullet strikes!");
	spFontDrawMiddle( screen->w >> 1, 2, 0, buffer, font );

	//Help
	draw_help();

	int b_alpha = bullet_alpha();
	if (b_alpha)
		spAddColorToTarget(EXPLOSION_COLOR,b_alpha);
	
	spFlip();
}

int direction_hold = 0;
#define DIRECTION_HOLD_TIME 200

int jump(int high)
{
	Sint32 dx = spSin(player[active_player]->activeHare->rotation);
	Sint32 dy = spCos(player[active_player]->activeHare->rotation);
	//if (circle_is_empty(player[active_player]->x+dx >> SP_ACCURACY,player[active_player]->y+dy >> SP_ACCURACY,6,0xDEAD))
	{
		if (high)
			player[active_player]->activeHare->hops = HIGH_HOPS_TIME;
		else
			player[active_player]->activeHare->hops = HOPS_TIME;
		player[active_player]->activeHare->high_hops = high;
	}
}

int min_d_not_me(int x,int y,pHare me)
{
	int min_d = LEVEL_WIDTH*LEVEL_WIDTH;
	int i;
	int my_d = spFixedToInt(me->x-x)*spFixedToInt(me->x-x)+
			   spFixedToInt(me->y-y)*spFixedToInt(me->y-y);
	if (my_d < 1024)
		return LEVEL_WIDTH*LEVEL_WIDTH;
	for (i = 0; i < player_count;i++)
	{
		if (player[i]->firstHare == NULL)
			continue;
		pHare hare = player[i]->firstHare;
		if (hare)
		do
		{							
				if (hare == me)
				{
					hare = hare->next;
					continue;
				}
				int d = spFixedToInt(hare->x-x)*spFixedToInt(hare->x-x)+
						spFixedToInt(hare->y-y)*spFixedToInt(hare->y-y);
				if (d < min_d)
					min_d = d;
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

void set_input()
{
	if (player[active_player]->computer)
		memset(input_states,0,sizeof(int)*12);
	else
	if (player[active_player]->local)
	{
		if (spGetInput()->button[MY_PRACTICE_4] == 0)
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
			if (spGetInput()->button[MY_BUTTON_L] != button_states[MY_BUTTON_L])
				input_states[INPUT_BUTTON_L] = spGetInput()->button[MY_BUTTON_L];
			if (spGetInput()->button[MY_BUTTON_R] != button_states[MY_BUTTON_R])
				input_states[INPUT_BUTTON_R] = spGetInput()->button[MY_BUTTON_R];
		}
		else
		{
			input_states[INPUT_AXIS_0_LEFT] = 0;
			input_states[INPUT_AXIS_0_RIGHT] = 0;
			input_states[INPUT_AXIS_1_LEFT] = 0;
			input_states[INPUT_AXIS_1_RIGHT] = 0;
		}
		if (spGetInput()->button[MY_PRACTICE_OK] != button_states[MY_PRACTICE_OK])
			input_states[INPUT_BUTTON_OK] = spGetInput()->button[MY_PRACTICE_OK];
		if (spGetInput()->button[MY_PRACTICE_3] != button_states[MY_PRACTICE_3])
			input_states[INPUT_BUTTON_3] = spGetInput()->button[MY_PRACTICE_3];
		if (spGetInput()->button[MY_PRACTICE_CANCEL] != button_states[MY_PRACTICE_CANCEL])
			input_states[INPUT_BUTTON_CANCEL] = spGetInput()->button[MY_PRACTICE_CANCEL];
		if (spGetInput()->button[MY_PRACTICE_4] != button_states[MY_PRACTICE_4])
			input_states[INPUT_BUTTON_4] = spGetInput()->button[MY_PRACTICE_4];
		if (spGetInput()->button[MY_BUTTON_START] != button_states[MY_BUTTON_START])
			input_states[INPUT_BUTTON_START] = spGetInput()->button[MY_BUTTON_START];
		if (spGetInput()->button[MY_BUTTON_SELECT] != button_states[MY_BUTTON_SELECT])
			input_states[INPUT_BUTTON_SELECT] = spGetInput()->button[MY_BUTTON_SELECT];
		memcpy(button_states,spGetInput()->button,sizeof(char)*SP_INPUT_BUTTON_COUNT);
		if (!hase_game->local)
		{
			add_ms_to_data(player[active_player]->time % 1000);
			if (player[active_player]->time % 1000 == 999)
			{
				printf("Pushing Second %i of player %s...\n",player[active_player]->time/1000,player[active_player]->name);
				push_game_thread(player[active_player],player[active_player]->time/1000,send_data);
				printf("Done\n");
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

int calc(Uint32 steps)
{
	if (spGetInput()->button[MY_BUTTON_SELECT] &&
	    spGetInput()->button[MY_BUTTON_START])
	{
		spGetInput()->button[MY_BUTTON_SELECT] = 0;
		spGetInput()->button[MY_BUTTON_START] = 0;
		return 1;
	}
	if (spGetInput()->button[MY_BUTTON_START])
	{
		spGetInput()->button[MY_BUTTON_START] = 0;
		help = 1-help;
	}
	if (spGetInput()->button[MY_PRACTICE_4])
	{
		if (spGetInput()->button[MY_BUTTON_L])
			zoom_d = -1;
		if (spGetInput()->button[MY_BUTTON_R])
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
		spSleep(200000);
	for (i = 0; i < steps; i++)
	{
		if (zoom_d == -1)
		{
			zoomAdjust -= 32;
			if (zoomAdjust < minZoom)
				zoomAdjust = minZoom;
			zoom = spMul(zoomAdjust,zoomAdjust);
			if ((zoomAdjust & 16383) == 0)
				zoom_d = 0;
		}
		else
		if (zoom_d == 1)
		{
			zoomAdjust += 32;
			if (zoomAdjust > maxZoom)
				zoomAdjust = maxZoom;
			zoom = spMul(zoomAdjust,zoomAdjust);
			if ((zoomAdjust & 16383) == 0)
				zoom_d = 0;
		}
		//Camera
		int destX,destY;
		destX = player[active_player]->activeHare->x;
		destY = player[active_player]->activeHare->y;
		if (firstBullet)
		{
			pBullet momBullet = firstBullet;
			int c = 1;
			while (momBullet)
			{
				destX += momBullet->x;
				destY += momBullet->y;
				momBullet = momBullet->next;
				c++;
			}
			destX /= c;
			destY /= c;
		}
		posX = (Sint64)posX*(Sint64)255+(Sint64)destX >> 8;
		posY = (Sint64)posY*(Sint64)255+(Sint64)destY >> 8;
		if (game_pause > 0)
			game_pause--;
		if (game_pause)
			continue;
		set_input();
		if (game_pause)
			continue;
		
		Sint32 goal = -player[active_player]->activeHare->rotation;
		if (goal < -SP_PI*3/2 && rotation > -SP_PI/2)
			rotation -= 2*SP_PI;
		if (goal > -SP_PI/2 && rotation < -SP_PI*3/2)
			rotation += 2*SP_PI;
		rotation = rotation*127/128+goal/128;
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
		check_next_player();
		if (weapon_points)
			countdown --;
		if (countdown < 0)
			next_player();
		if (player[active_player]->computer)
		{
			//AI
			if (weapon_points)
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
								int d = min_d_not_me(x,y,player[active_player]->activeHare);
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
								if (weapon_points > 0)
								{
									weapon_points--;
									shootBullet(player[active_player]->activeHare->x,player[active_player]->activeHare->y,player[active_player]->activeHare->w_direction+player[active_player]->activeHare->rotation+SP_PI,player[active_player]->activeHare->w_power/2,player[active_player]->activeHare->direction?1:-1,player[active_player]);
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
			if (input_states[INPUT_BUTTON_CANCEL])
			{
				//Shoot!
				if (weapon_points > 0)
				{
					weapon_points--;
					input_states[INPUT_BUTTON_CANCEL] = 0;
					shootBullet(player[active_player]->activeHare->x,player[active_player]->activeHare->y,player[active_player]->activeHare->w_direction+player[active_player]->activeHare->rotation+SP_PI,player[active_player]->activeHare->w_power/2,player[active_player]->activeHare->direction?1:-1,player[active_player]);
				}
			}
		}
		if (firstBullet == NULL && weapon_points == 0)
		{
			if (extra_time == 0)
				extra_time = 5000;
			else
			if (extra_time == 1)
				next_player();
			else
				extra_time--;
		}
	}
	return result;
}

int hase(void ( *resize )( Uint16 w, Uint16 h ),pGame game,pPlayer me_list)
{
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
	arrow = spLoadSurface("./data/gravity.png");
	weapon = spLoadSurface("./data/weapon.png");
	bullet = spLoadSurface("./data/bullet.png");
	gravity_surface = spCreateSurface( GRAVITY_DENSITY << GRAVITY_RESOLUTION+1, GRAVITY_DENSITY << GRAVITY_RESOLUTION+1);
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
	zoomAdjust = spSqrt(spGetSizeFactor());
	minZoom = spSqrt(spGetSizeFactor()/8);
	maxZoom = spSqrt(spGetSizeFactor()*4);
	zoom = spMul(zoomAdjust,zoomAdjust);
	zoom_d = 0;
	countdown = hase_game->seconds_per_turn*1000;
	alive_count = player_count;
	memset(input_states,0,sizeof(int)*12);
	memset(button_states,0,sizeof(char)*SP_INPUT_BUTTON_COUNT);
	memset(send_data,0,1536*sizeof(char));
	game_pause = 0;
	
	int result = spLoop(draw,calc,10,resize,NULL);

	stop_thread();
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
	spDeleteSurface(arrow);
	spDeleteSurface(weapon);
	spDeleteSurface(bullet);
	spDeleteSurface(level);
	spDeleteSurface(level_original);
	spDeleteSurface(gravity_surface);
	spResetButtonsState();
	return 0;
}
