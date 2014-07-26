#define HOPS_TIME 200
#define HIGH_HOPS_TIME 403
#define MAX_HEALTH 256
#define AI_MAX_TRIES 64
#define AI_TRIES_EVERY_MS 32
int ai_shoot_tries = 0;
int last_ai_try = 0;

#include "message.h"

int lastAIDistance = 100000000;

int active_player = 0;
int player_count;
pPlayer *player;

int circle_is_empty(int x, int y, int r,int except)
{
	int a,b;
	int start_a = x-r;
	if (start_a < 0)
		start_a = 0;
	int end_a = x+r+1;
	if (end_a > LEVEL_WIDTH)
		end_a = LEVEL_WIDTH;
	for (a = start_a; a < end_a; a++)
	{
		int start_b = y-r;
		if (start_b < 0)
			start_b = 0;
		int end_b = y+r+1;
		if (end_b > LEVEL_HEIGHT)
			end_b = LEVEL_HEIGHT;
		for (b = start_b; b < end_b; b++)
		if (a!=x && b!=y && (a-x)*(a-x)+(b-y)*(b-y) <= r*r)
		{
			if (level_pixel[a+b*LEVEL_WIDTH] != SP_ALPHA_COLOR)
				return 0;
		}
	}
	
	int i;
	if (except < 0xDEAD)
		for (i = 0; i < player_count; i++)
		{
			if (player[i]->health == 0)
				continue;
			if (i == except)
				continue;
			int px = player[i]->x >> SP_ACCURACY;
			int py = player[i]->y >> SP_ACCURACY;
			int d = (x-px)*(x-px)+(y-py)*(y-py);
			if (d <= (r+8)*(r+8))
				return 0;
		}
	return 1;
}

Sint32 gravitation_x(int x,int y)
{
	int gx1 = x - (1 << GRAVITY_RESOLUTION - 1) >> GRAVITY_RESOLUTION;
	int gy1 = y - (1 << GRAVITY_RESOLUTION - 1) >> GRAVITY_RESOLUTION;
	int gx2 = gx1+1;
	int gy2 = gy1+1;
	int rx = x - (gx1 << GRAVITY_RESOLUTION);
	int ry = y - (gy1 << GRAVITY_RESOLUTION);
	if (gx1 < 0 || gy1 < 0 || gx1 >= (LEVEL_WIDTH >> GRAVITY_RESOLUTION) || gy1 >= (LEVEL_HEIGHT >> GRAVITY_RESOLUTION))
		return 0;
	if (gx2 < 0 || gy2 < 0 || gx2 >= (LEVEL_WIDTH >> GRAVITY_RESOLUTION) || gy2 >= (LEVEL_HEIGHT >> GRAVITY_RESOLUTION))
		return 0;
	int g1 = gravity[gx2+gy1*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x * rx + gravity[gx1+gy1*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x * ((1<<GRAVITY_RESOLUTION)-rx) >> GRAVITY_RESOLUTION;
	int g2 = gravity[gx2+gy2*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x * rx + gravity[gx1+gy2*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x * ((1<<GRAVITY_RESOLUTION)-rx) >> GRAVITY_RESOLUTION;
	return g2 * ry + g1 * ((1<<GRAVITY_RESOLUTION)-ry) >> GRAVITY_RESOLUTION;
}

Sint32 gravitation_y(int x,int y)
{
	int gx1 = x - (1 << GRAVITY_RESOLUTION - 1) >> GRAVITY_RESOLUTION;
	int gy1 = y - (1 << GRAVITY_RESOLUTION - 1) >> GRAVITY_RESOLUTION;
	int gx2 = gx1+1;
	int gy2 = gy1+1;
	int rx = x - (gx1 << GRAVITY_RESOLUTION);
	int ry = y - (gy1 << GRAVITY_RESOLUTION);
	if (gx1 < 0 || gy1 < 0 || gx1 >= (LEVEL_WIDTH >> GRAVITY_RESOLUTION) || gy1 >= (LEVEL_HEIGHT >> GRAVITY_RESOLUTION))
		return 0;
	if (gx2 < 0 || gy2 < 0 || gx2 >= (LEVEL_WIDTH >> GRAVITY_RESOLUTION) || gy2 >= (LEVEL_HEIGHT >> GRAVITY_RESOLUTION))
		return 0;
	int g1 = gravity[gx2+gy1*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y * rx + gravity[gx1+gy1*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y * ((1<<GRAVITY_RESOLUTION)-rx) >> GRAVITY_RESOLUTION;
	int g2 = gravity[gx2+gy2*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y * rx + gravity[gx1+gy2*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y * ((1<<GRAVITY_RESOLUTION)-rx) >> GRAVITY_RESOLUTION;
	return g2 * ry + g1 * ((1<<GRAVITY_RESOLUTION)-ry) >> GRAVITY_RESOLUTION;
}

Sint32 gravitation_force(int x,int y)
{
	int grav_x = gravitation_x(x,y);
	int grav_y = gravitation_y(x,y);
	if (grav_x == 0 && grav_y == 0)
		return 0;
	return spSqrt(spMul(grav_x,grav_x)+spMul(grav_y,grav_y));
}

void update_player()
{
	int j;
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->health == 0)
			continue;
		if (player[j]->hops > 0)
		{
			player[j]->hops --;
			if (player[j]->hops <= 0)
			{
				Sint32 dx = spSin(player[j]->rotation);
				Sint32 dy = spCos(player[j]->rotation);
				Sint32 ox = player[j]->x;
				Sint32 oy = player[j]->y;
				int k;
				for (k = 1; k <= 16; k++)
				{
					if (circle_is_empty(spFixedToInt(player[j]->x+k*dx),spFixedToInt(player[j]->y-k*dy),8,j))
					{
						player[j]->x += k*dx;
						player[j]->y -= k*dy;
						Sint32 angle = SP_PI/3;
						Sint32 divisor = 8;
						if (player[j]->high_hops)
						{
							angle = SP_PI/10;
							divisor = 6;
						}
						if (player[j]->direction)
						{
							player[j]->dx += spSin(player[j]->rotation+angle)/divisor;
							player[j]->dy -= spCos(player[j]->rotation+angle)/divisor;
						}
						else
						{
							player[j]->dx += spSin(player[j]->rotation-angle)/divisor;
							player[j]->dy -= spCos(player[j]->rotation-angle)/divisor;
						}
						break;
					}
					if (k == 16)
					{
						k = -1;
						printf("Using special magic...\n");
					}
				}
			}
		}
		player[j]->rotation = 0;
		player[j]->bums = 0;
		Sint32 force = gravitation_force(spFixedToInt(player[j]->x),spFixedToInt(player[j]->y));
		if (force)
		{
			Sint32 ac = spDiv(-gravitation_y(spFixedToInt(player[j]->x),spFixedToInt(player[j]->y)),force);
			if (ac < -SP_ONE)
				ac = -SP_ONE;
			if (ac > SP_ONE)
				ac = SP_ONE;
			player[j]->rotation = -spAcos(ac);
			if (-gravitation_x(spFixedToInt(player[j]->x),spFixedToInt(player[j]->y)) <= 0)
				player[j]->rotation = 2*SP_PI-player[j]->rotation;
			while (player[j]->rotation < 0)
				player[j]->rotation += 2*SP_PI;
			while (player[j]->rotation >= 2*SP_PI)
				player[j]->rotation -= 2*SP_PI;
		}
	}
}

void update_player_sprite(int steps)
{
	if (player[active_player]->hops > 0)
	{
		if (player[active_player]->high_hops)
		{
			if (player[active_player]->direction == 0)
				spSelectSprite(player[active_player]->hase,"high jump left");
			else
				spSelectSprite(player[active_player]->hase,"high jump right");
		}
		else
		{
			if (player[active_player]->direction == 0)
				spSelectSprite(player[active_player]->hase,"jump left");
			else
				spSelectSprite(player[active_player]->hase,"jump right");
		}
	}
	else
	{
		if (player[active_player]->direction == 0)
			spSelectSprite(player[active_player]->hase,"stand left");
		else
			spSelectSprite(player[active_player]->hase,"stand right");
	}
	spUpdateSprite(spActiveSprite(player[active_player]->hase),steps);
}	

int next_player_go = 0;

void next_player()
{
	next_player_go = 1;
}

void stop_thread()
{
	if (!hase_game->local && active_player >= 0)
	{
		if (!player[active_player]->computer)
		{
			if (player[active_player]->local)
			{
				printf("Ending Push Thread for player %s (s: %i)\n",player[active_player]->name,player[active_player]->time/1000);
				char buffer[320];
				sprintf(buffer,"Finishing sending turn data\nfor player %s...",player[active_player]->name);
				message(font,hase_resize,buffer,0,NULL);
				message_draw();
				spFlip();
				push_game_thread(player[active_player],player[active_player]->time/1000,send_data);
				memset(send_data,0,sizeof(char)*1536);
				end_push_thread();
			}
			else
			{
				printf("Ending Pull Thread for player %s\n",player[active_player]->name);
				end_pull_thread(player[active_player]);
			}
		}
		printf("Setting player time from %i to %i\n",player[active_player]->time,((player[active_player]->time+999)/1000)*1000);
		player[active_player]->time = ((player[active_player]->time+999)/1000)*1000;
	}	
}

void start_thread()
{
	if (!hase_game->local)
	{
		if (!player[active_player]->computer)
		{
			memset(send_data,0,1536*sizeof(char));
			if (player[active_player]->local)
			{
				printf("Starting Push Thread for player %s\n",player[active_player]->name);
				start_push_thread();
			}
			else
			{
				printf("Starting Pull Thread for player %s\n",player[active_player]->name);
				start_pull_thread(player[active_player]);
			}
		}
	}
}

void real_next_player()
{
	stop_thread();
	ai_shoot_tries = 0;
	last_ai_try = 0;
	lastAIDistance = 100000000;
	do
	{
		active_player = (active_player+1)%player_count;
	}
	while (player[active_player]->health == 0);
	player[active_player]->shoot = 0;
	player[active_player]->bullet = NULL;
	if (player[active_player]->computer)
		player[active_player]->direction = spRand()&1;
	countdown = hase_game->seconds_per_turn*1000;
	player[active_player]->hops = 0;
	player[active_player]->high_hops = 0;
	memset(input_states,0,sizeof(int)*12);
	start_thread();
}

void check_next_player()
{
	if (next_player_go && bullet_alpha() == 0)
	{
		next_player_go = 0;
		real_next_player();
	}
}

void init_player(pPlayer player_list,int pc)
{
	next_player_go = 0;
	player_count = pc;
	player = (pPlayer*)malloc(sizeof(pPlayer)*pc);
	while (player_list)
	{
		printf("Setting player %i\n",player_list->position_in_game);
		player[player_list->position_in_game] = player_list;
		player_list = player_list->next;
	}
	int i;
	for (i = 0; i < player_count; i ++)
	{
		player[i]->time = 0;
		player[i]->direction = 0;
		player[i]->w_direction = SP_ONE/2;
		player[i]->w_power = SP_ONE/2;
		player[i]->hops = 0;
		player[i]->high_hops = 0;
		int x,y;
		while (1)
		{
			x = spRand()%LEVEL_WIDTH;
			y = spRand()%LEVEL_HEIGHT;
			printf("Tried %i %i... ",x,y);
			if (circle_is_empty(x,y,16,i) && gravitation_force(x,y)/32768)
				break;
			printf("NOT!\n");
		}
		printf("Fine.\n");
		player[i]->x = x << SP_ACCURACY;
		player[i]->y = y << SP_ACCURACY;
		player[i]->dx = 0;
		player[i]->dy = 0;
		posX = player[i]->x;
		posY = player[i]->y;
		update_player(0);
		rotation = -player[i]->rotation;
		player[i]->health = MAX_HEALTH;
		char buffer[256];
		sprintf(buffer,"./data/hase%i.ssc",spRand()%10+1);
		player[i]->hase = spLoadSpriteCollection(buffer,NULL);
	}
	active_player = 0;
	player[active_player]->shoot = 0;
	player[active_player]->bullet = NULL;
	posX = player[active_player]->x;
	posY = player[active_player]->y;
	ai_shoot_tries = 0;
	last_ai_try = 0;
	start_thread();
}

