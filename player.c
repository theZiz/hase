#define HOPS_TIME 200
#define HIGH_HOPS_TIME 403
#define MAX_HEALTH 256
#define AI_MAX_TRIES 512
#define AI_TRIES_PER_FRAME 16
int ai_shoot_tries = 0;

int lastAIDistance = 100000000;

typedef struct sBullet *pBullet;


typedef struct sPlayer *pPlayer;
typedef struct sPlayer
{
	int direction;
	int w_direction;
	int w_power;
	Sint32 x,y;
	Sint32 dx,dy;
	Sint32 rotation;
	int bums;
	int hops;
	int high_hops;
	int health;
	spSpriteCollectionPointer hase;
	int shoot;
	pBullet bullet;
} tPlayer;

int active_player = 0;
tPlayer player[2];

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
		for (i = 0; i < 2; i++)
		{
			if (i == except)
				continue;
			int px = player[i].x >> SP_ACCURACY;
			int py = player[i].y >> SP_ACCURACY;
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

void update_player(int steps)
{
	if (player[active_player].hops > 0)
	{
		player[active_player].hops -= steps;
		if (player[active_player].hops <= 0)
		{
			Sint32 dx = spSin(player[active_player].rotation);
			Sint32 dy = spCos(player[active_player].rotation);
			player[active_player].x += dx;
			player[active_player].y -= dy;
			Sint32 angle = SP_PI/3;
			Sint32 divisor = 8;
			if (player[active_player].high_hops)
			{
				angle = SP_PI/10;
				divisor = 6;
			}
			if (player[active_player].direction)
			{
				player[active_player].dx += spSin(player[active_player].rotation+angle)/divisor;
				player[active_player].dy -= spCos(player[active_player].rotation+angle)/divisor;
			}
			else
			{
				player[active_player].dx += spSin(player[active_player].rotation-angle)/divisor;
				player[active_player].dy -= spCos(player[active_player].rotation-angle)/divisor;
			}			
		}
	}
	int j;
	for (j = 0; j < 2; j++)
	{
		player[j].rotation = 0;
		player[j].bums = 0;
		Sint32 force = gravitation_force(player[j].x >> SP_ACCURACY,player[j].y >> SP_ACCURACY);
		if (force)
		{
			Sint32 ac = spDiv(-gravitation_y(player[j].x >> SP_ACCURACY,player[j].y >> SP_ACCURACY),force);
			if (ac < -SP_ONE)
				ac = -SP_ONE;
			if (ac > SP_ONE)
				ac = SP_ONE;
			player[j].rotation = -spAcos(ac);
			if (-gravitation_x(player[j].x >> SP_ACCURACY,player[j].y >> SP_ACCURACY) <= 0)
				player[j].rotation = 2*SP_PI-player[j].rotation;
			while (player[j].rotation < 0)
				player[j].rotation += 2*SP_PI;
			while (player[j].rotation >= 2*SP_PI)
				player[j].rotation -= 2*SP_PI;
		}
	}
}

void update_player_sprite(int steps)
{
	if (player[active_player].hops > 0)
	{
		if (player[active_player].high_hops)
		{
			if (player[active_player].direction == 0)
				spSelectSprite(player[active_player].hase,"high jump left");
			else
				spSelectSprite(player[active_player].hase,"high jump right");
		}
		else
		{
			if (player[active_player].direction == 0)
				spSelectSprite(player[active_player].hase,"jump left");
			else
				spSelectSprite(player[active_player].hase,"jump right");
		}
	}
	else
	{
		if (player[active_player].direction == 0)
			spSelectSprite(player[active_player].hase,"stand left");
		else
			spSelectSprite(player[active_player].hase,"stand right");
	}
	spUpdateSprite(spActiveSprite(player[active_player].hase),steps);
}	

void init_player()
{
	int i;
	for (i = 0; i < 2; i ++)
	{
		player[i].direction = 0;
		player[i].w_direction = SP_ONE/2;
		player[i].w_power = SP_ONE/2;
		player[i].hops = 0;
		player[i].high_hops = 0;
		int x,y;
		while (1)
		{
			x = rand()%LEVEL_WIDTH;
			y = rand()%LEVEL_HEIGHT;
			printf("Tried %i %i...",x,y);
			if (circle_is_empty(x,y,16,i) && gravitation_force(x,y)/32768)
				break;
			printf("NOT!\n");
		}
		printf("Fine.\n");
		player[i].x = x << SP_ACCURACY;
		player[i].y = y << SP_ACCURACY;
		player[i].dx = 0;
		player[i].dy = 0;
		posX = player[i].x;
		posY = player[i].y;
		update_player(0);
		rotation = -player[i].rotation;
		player[i].health = MAX_HEALTH;
		switch (i)
		{
			case 0:	player[i].hase = spLoadSpriteCollection("./data/hase.ssc",NULL); break;
			case 1:	player[i].hase = spLoadSpriteCollection("./data/hase2.ssc",NULL); break;
		}
	}
	active_player = rand()%2;
	player[active_player].shoot = 0;
	player[active_player].bullet = NULL;
	posX = player[active_player].x;
	posY = player[active_player].y;
	ai_shoot_tries = 0;
}

void next_player()
{
	ai_shoot_tries = 0;
	lastAIDistance = 100000000;
	active_player = 1-active_player;
	player[active_player].shoot = 0;
	player[active_player].bullet = NULL;
	if (active_player == 1)
		player[active_player].direction = rand()&1;
	countdown = COUNT_DOWN;
	player[active_player].hops = 0;
	player[active_player].high_hops = 0;
}
