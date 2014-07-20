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
Sint32 zoomAdjust;
Sint32 minZoom,maxZoom;
int help = 0;
int countdown;

int power_pressed = 0;
int direction_pressed = 0;
int alive_count;

int game_pause = 0;

#define INPUT_AXIS_0_LEFT 0
#define INPUT_AXIS_0_RIGHT 1
#define INPUT_AXIS_1_LEFT 2
#define INPUT_AXIS_1_RIGHT 3
#define INPUT_BUTTON_LEFT 4
#define INPUT_BUTTON_UP 5
#define INPUT_BUTTON_RIGHT 6
#define INPUT_BUTTON_DOWN 7
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
	posX = player[active_player]->x;
	posY = player[active_player]->y;	
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
	if (player[active_player]->w_power)
	{
		Sint32 x = spCos(player[active_player]->w_direction)*(-8-spFixedToInt(16*player[active_player]->w_power));
		Sint32 y = spSin(player[active_player]->w_direction)*(-8-spFixedToInt(16*player[active_player]->w_power));
		spRotozoomSurface(screen->w/2+(spMul(player[active_player]->x-posX+x,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY+y,zoom) >> SP_ACCURACY),0,arrow,spMul(zoom,spGetSizeFactor())/8,spMul(player[active_player]->w_power,spMul(zoom,spGetSizeFactor()))/4,player[active_player]->w_direction-SP_PI/2);
	}

	//Bullets
	drawBullets();
	
	//Weapon
	spRotozoomSurface(screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY,zoom) >> SP_ACCURACY),0,weapon,zoom/2,zoom/2,player[active_player]->w_direction);
	
	//Player
	spSpritePointer sprite = spActiveSprite(player[active_player]->hase);
	spSetSpriteZoom(sprite,zoom/2,zoom/2);
	spSetSpriteRotation(sprite,0);
	spDrawSprite(screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY,zoom) >> SP_ACCURACY),0,sprite);

	//Health circle
	spEllipse(screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY-spIntToFixed(14),zoom) >> SP_ACCURACY),0,
	          spFixedToInt(zoom*6)+1,
	          spFixedToInt(zoom*6)+1,spGetRGB(255,0,0));	
	spEllipse(screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY-spIntToFixed(14),zoom) >> SP_ACCURACY),0,
	          spFixedToInt(zoom*6*(player[active_player]->health)/MAX_HEALTH)+1,
	          spFixedToInt(zoom*6*(player[active_player]->health)/MAX_HEALTH)+1,spGetRGB(0,255,0));
	//spFontDrawMiddle( screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY-spIntToFixed(10),zoom) >> SP_ACCURACY)-font->maxheight/2,0,"100", font );
	if (player[active_player]->computer)
		sprintf(buffer,"%s (AI)",player[active_player]->name);
	else
		sprintf(buffer,"%s",player[active_player]->name);
	spSetBlending( SP_ONE*2/3 );
	spFontDrawMiddle( screen->w/2+(spMul(player[active_player]->x-posX,zoom) >> SP_ACCURACY),screen->h/2+(spMul(player[active_player]->y-posY-spIntToFixed(14),zoom) >> SP_ACCURACY),0,buffer, font );
	spSetBlending( SP_ONE );
	
	//Not the player:
	int not_active_player;
	for (not_active_player = 0; not_active_player < player_count; not_active_player++)
	{
		if (not_active_player == active_player)
			continue;
		if (player[not_active_player]->health == 0)
			continue;
		sprite = spActiveSprite(player[not_active_player]->hase);
		spSetSpriteZoom(sprite,zoom/2,zoom/2);
		spSetSpriteRotation(sprite,+rotation+player[not_active_player]->rotation);
		Sint32 ox = spMul(player[not_active_player]->x-posX,zoom);
		Sint32 oy = spMul(player[not_active_player]->y-posY,zoom);
		Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
		Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
		spDrawSprite(screen->w/2+x,screen->h/2+y,0,sprite);
		//Health circle
		ox = spMul(player[not_active_player]->x-posX-14*-spSin(player[not_active_player]->rotation),zoom);
		oy = spMul(player[not_active_player]->y-posY-14* spCos(player[not_active_player]->rotation),zoom);
		x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
		y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
		spEllipse(screen->w/2+x,screen->h/2+y,0,
				  spFixedToInt(zoom*6)+1,
				  spFixedToInt(zoom*6)+1,spGetRGB(255,0,0));	
		spEllipse(screen->w/2+x,screen->h/2+y,0,
				  spFixedToInt(zoom*6*(player[not_active_player]->health)/MAX_HEALTH)+1,
				  spFixedToInt(zoom*6*(player[not_active_player]->health)/MAX_HEALTH)+1,spGetRGB(0,255,0));
		if (player[not_active_player]->computer)
			sprintf(buffer,"%s (AI)",player[not_active_player]->name);
		else
			sprintf(buffer,"%s",player[not_active_player]->name);
		spSetBlending( SP_ONE*2/3 );
		spFontDrawMiddle( screen->w/2+x,screen->h/2+y,0,buffer, font );
		spSetBlending( SP_ONE );
	}
	//Trace
	if (player[active_player]->shoot == 0)
		drawTrace();
	
	//Help
	draw_help();
	
	//HID
	sprintf(buffer,"FPS: %i",spGetFPS());
	spFontDrawRight( screen->w-1, screen->h-1-font->maxheight, 0, buffer, font );
	sprintf(buffer,"Power: %i %%",player[active_player]->w_power*100/SP_ONE);
	spFontDraw( 2, 2, 0, buffer, font );	
	sprintf(buffer,"%i / %i\n",alive_count,hase_game->player_count);	
	spFontDrawRight( screen->w-2, 2, 0, buffer, font );
	sprintf(buffer,"%i seconds left",countdown / 1000);
	spFontDrawMiddle( screen->w >> 1, 2, 0, buffer, font );
	int b_alpha = bullet_alpha();
	if (b_alpha)
		spAddColorToTarget(EXPLOSION_COLOR,b_alpha);
	if (game_pause)
		message_draw();
	spFlip();
}

int direction_hold = 0;
#define DIRECTION_HOLD_TIME 200

int jump(int high)
{
	Sint32 dx = spSin(player[active_player]->rotation);
	Sint32 dy = spCos(player[active_player]->rotation);
	//if (circle_is_empty(player[active_player]->x+dx >> SP_ACCURACY,player[active_player]->y+dy >> SP_ACCURACY,6,0xDEAD))
	{
		if (high)
			player[active_player]->hops = HIGH_HOPS_TIME;
		else
			player[active_player]->hops = HOPS_TIME;
		player[active_player]->high_hops = high;
	}
}

int min_d_not_me(int x,int y,int me)
{
	int min_d = 10000;
	int i;
	int my_d = spFixedToInt(player[me]->x-x)*spFixedToInt(player[me]->x-x)+
			   spFixedToInt(player[me]->y-y)*spFixedToInt(player[me]->y-y);
	if (my_d < 256)
		min_d*=2;
	else
		for (i = 0; i < 0;i++)
		{
			if (i == me)
				continue;
			int d = spFixedToInt(player[i]->x-x)*spFixedToInt(player[i]->x-x)+
					spFixedToInt(player[i]->y-y)*spFixedToInt(player[i]->y-y);
			if (d < min_d)
				min_d = d;
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
		if (spGetInput()->button[SP_BUTTON_LEFT] != button_states[SP_BUTTON_LEFT])
			input_states[INPUT_BUTTON_LEFT] = spGetInput()->button[SP_BUTTON_LEFT];
		if (spGetInput()->button[SP_BUTTON_UP] != button_states[SP_BUTTON_UP])
			input_states[INPUT_BUTTON_UP] = spGetInput()->button[SP_BUTTON_UP];
		if (spGetInput()->button[SP_BUTTON_RIGHT] != button_states[SP_BUTTON_RIGHT])
			input_states[INPUT_BUTTON_RIGHT] = spGetInput()->button[SP_BUTTON_RIGHT];
		if (spGetInput()->button[SP_BUTTON_DOWN] != button_states[SP_BUTTON_DOWN])
			input_states[INPUT_BUTTON_DOWN] = spGetInput()->button[SP_BUTTON_DOWN];
		if (spGetInput()->button[SP_BUTTON_L] != button_states[SP_BUTTON_L])
			input_states[INPUT_BUTTON_L] = spGetInput()->button[SP_BUTTON_L];
		if (spGetInput()->button[SP_BUTTON_R] != button_states[SP_BUTTON_R])
			input_states[INPUT_BUTTON_R] = spGetInput()->button[SP_BUTTON_R];
		if (spGetInput()->button[SP_BUTTON_START] != button_states[SP_BUTTON_START])
			input_states[INPUT_BUTTON_START] = spGetInput()->button[SP_BUTTON_START];
		if (spGetInput()->button[SP_BUTTON_SELECT] != button_states[SP_BUTTON_SELECT])
			input_states[INPUT_BUTTON_SELECT] = spGetInput()->button[SP_BUTTON_SELECT];
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
				message(font,hase_resize,buffer,0,NULL);
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
	int i;
	update_player_sprite(steps);
	int result = 0;
	if (game_pause)
		spSleep(200000);
	for (i = 0; i < steps; i++)
	{
		if (game_pause > 0)
			game_pause--;
		if (game_pause)
			continue;
		set_input();
		if (game_pause)
			continue;
		
		Sint32 goal = -player[active_player]->rotation;
		if (goal < -SP_PI*3/2 && rotation > -SP_PI/2)
			rotation -= 2*SP_PI;
		if (goal > -SP_PI/2 && rotation < -SP_PI*3/2)
			rotation += 2*SP_PI;
		rotation = rotation*127/128+goal/128;
		update_player(1);
		int res = do_physics(1);
		if (res == 1)
			result = 1;
		if (res)
		{
			i = steps;
			continue;
		}
		if (bullet_alpha() > 0)
			continue;
		check_next_player();
		if (player[active_player]->shoot == 0)
			countdown --;
		if (countdown < 0)
			next_player();
		if (input_states[INPUT_BUTTON_START])
		{
			input_states[INPUT_BUTTON_START] = 0;
			help = 1-help;
		}
		if (player[active_player]->computer)
		{
			//AI
			if (player[active_player]->shoot == 0)
			{
				if (player[active_player]->bums)
				{
					int j;
					for (j = 0; j < AI_TRIES_PER_FRAME; j++)
					{
						ai_shoot_tries++;
						if (ai_shoot_tries < AI_MAX_TRIES)
						{
							//Lets first try...
							int x = player[active_player]->x;
							int y = player[active_player]->y;
							int w_d = rand()%(2*SP_PI);
							int w_p = rand()%SP_ONE;
							lastPoint(&x,&y,player[active_player]->rotation+w_d+SP_PI,w_p/2);
							int d = min_d_not_me(x,y,active_player);
							if (d < lastAIDistance)
							{
								lastAIDistance = d;
								player[active_player]->w_direction = w_d;
								player[active_player]->w_power = w_p;
							}
						}
						else
						{
							//Shoot!
							player[active_player]->shoot = 1;
							player[active_player]->bullet = shootBullet(player[active_player]->x,player[active_player]->y,player[active_player]->w_direction+player[active_player]->rotation+SP_PI,player[active_player]->w_power/2,player[active_player]->direction?1:-1);
							break;
						}
					}
				}
			}
			else
			{
				//RUNNING!
				if (player[active_player]->bums && player[active_player]->hops <= 0)
					jump((rand()%4==0)?1:0);
			}
		}
		else
		{
			//not AI
			if (input_states[INPUT_BUTTON_LEFT] && player[active_player]->bums && player[active_player]->hops <= 0)
			{
				jump(1);
			}
			if (input_states[INPUT_AXIS_0_LEFT])
			{
				if (player[active_player]->direction == 1)
				{
					player[active_player]->direction = 0;
					player[active_player]->w_direction = SP_PI-player[active_player]->w_direction;
					direction_hold = 0;
				}
				direction_hold++;
				if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player]->bums && player[active_player]->hops <= 0)
					jump(0);
			}
			else
			if (input_states[INPUT_AXIS_0_RIGHT])
			{
				if (player[active_player]->direction == 0)
				{
					player[active_player]->direction = 1;
					player[active_player]->w_direction = SP_PI-player[active_player]->w_direction;
					direction_hold = 0;
				}
				direction_hold++;
				if (direction_hold >= DIRECTION_HOLD_TIME && player[active_player]->bums && player[active_player]->hops <= 0)
					jump(0);
			}
			else
				direction_hold = 0;
			
			if (input_states[INPUT_AXIS_1_LEFT])
			{
				direction_pressed += SP_ONE/20;
				if (direction_pressed >= 128*SP_ONE)
					direction_pressed = 128*SP_ONE;
				if (player[active_player]->direction == 0)
					player[active_player]->w_direction += direction_pressed >> SP_ACCURACY;
				else
					player[active_player]->w_direction -= direction_pressed >> SP_ACCURACY;
			}
			else
			if (input_states[INPUT_AXIS_1_RIGHT])
			{
				direction_pressed += SP_ONE/20;
				if (direction_pressed >= 128*SP_ONE)
					direction_pressed = 128*SP_ONE;
				if (player[active_player]->direction == 0)
					player[active_player]->w_direction -= direction_pressed >> SP_ACCURACY;
				else
					player[active_player]->w_direction += direction_pressed >> SP_ACCURACY;
			}
			else
				direction_pressed = 0;

			if (input_states[INPUT_BUTTON_UP])
			{
				power_pressed += SP_ONE/100;
				player[active_player]->w_power += power_pressed >> SP_ACCURACY;
				if (player[active_player]->w_power >= SP_ONE)
					player[active_player]->w_power = SP_ONE;
			}
			else
			if (input_states[INPUT_BUTTON_DOWN])
			{
				power_pressed += SP_ONE/100;
				player[active_player]->w_power -= power_pressed >> SP_ACCURACY;
				if (player[active_player]->w_power < 0)
					player[active_player]->w_power = 0;
			}
			else
				power_pressed = 0;
			if (input_states[INPUT_BUTTON_RIGHT] && player[active_player]->shoot == 0)
			{
				//Shoot!
				player[active_player]->shoot = 1;
				input_states[INPUT_BUTTON_RIGHT] = 0;
				player[active_player]->bullet = shootBullet(player[active_player]->x,player[active_player]->y,player[active_player]->w_direction+player[active_player]->rotation+SP_PI,player[active_player]->w_power/2,player[active_player]->direction?1:-1);
			}
		}
	}
	if (player[active_player]->shoot == 0)
		updateTrace();
	if (input_states[INPUT_BUTTON_SELECT])
		return 1;
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
	srand(0);
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
	init_player(hase_player_list,game->player_count);
	updateTrace();
	zoomAdjust = spSqrt(spGetSizeFactor());
	minZoom = spSqrt(spGetSizeFactor()/8);
	maxZoom = spSqrt(spGetSizeFactor()*4);
	zoom = spMul(zoomAdjust,zoomAdjust);
	countdown = hase_game->seconds_per_turn*1000;
	alive_count = player_count;
	memset(input_states,0,sizeof(int)*12);
	memset(button_states,0,sizeof(char)*SP_INPUT_BUTTON_COUNT);
	memset(send_data,0,1536*sizeof(char));
	game_pause = 0;
	
	spLoop(draw,calc,10,resize,NULL);
	
	deleteAllBullets();
	free_gravity();
	int i;
	for (i = 0; i < player_count; i++)
		spDeleteSpriteCollection(player[i]->hase,0);
	spDeleteSurface(arrow);
	spDeleteSurface(weapon);
	spDeleteSurface(bullet);
	spDeleteSurface(level);
	spDeleteSurface(level_original);
	spDeleteSurface(gravity_surface);
	return 0;
}
