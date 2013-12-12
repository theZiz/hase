#define HOPS_TIME 200
#define HIGH_HOPS_TIME 403
#define MAX_HEALTH 256

spSpriteCollectionPointer hase;
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
} tPlayer;

tPlayer player;

int circle_is_empty(int x, int y, int r)
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
	if (player.hops > 0)
	{
		player.hops -= steps;
		if (player.hops <= 0)
		{
			Sint32 dx = spSin(player.rotation);
			Sint32 dy = spCos(player.rotation);
			player.x += dx;
			player.y -= dy;
			Sint32 angle = SP_PI/3;
			Sint32 divisor = 8;
			if (player.high_hops)
			{
				angle = SP_PI/10;
				divisor = 6;
			}
			if (player.direction)
			{
				player.dx += spSin(player.rotation+angle)/divisor;
				player.dy -= spCos(player.rotation+angle)/divisor;
			}
			else
			{
				player.dx += spSin(player.rotation-angle)/divisor;
				player.dy -= spCos(player.rotation-angle)/divisor;
			}			
		}
	}
	
	player.rotation = 0;
	player.bums = 0;
	Sint32 force = gravitation_force(player.x >> SP_ACCURACY,player.y >> SP_ACCURACY);
	if (force)
	{
		Sint32 ac = spDiv(-gravitation_y(player.x >> SP_ACCURACY,player.y >> SP_ACCURACY),force);
		if (ac < -SP_ONE)
			ac = -SP_ONE;
		if (ac > SP_ONE)
			ac = SP_ONE;
		player.rotation = -spAcos(ac);
		if (-gravitation_x(player.x >> SP_ACCURACY,player.y >> SP_ACCURACY) <= 0)
			player.rotation = 2*SP_PI-player.rotation;
		while (player.rotation < 0)
			player.rotation += 2*SP_PI;
		while (player.rotation >= 2*SP_PI)
			player.rotation -= 2*SP_PI;
	}
}

void update_player_sprite(int steps)
{
	if (player.hops > 0)
	{
		if (player.high_hops)
		{
			if (player.direction == 0)
				spSelectSprite(hase,"high jump left");
			else
				spSelectSprite(hase,"high jump right");
		}
		else
		{
			if (player.direction == 0)
				spSelectSprite(hase,"jump left");
			else
				spSelectSprite(hase,"jump right");
		}
	}
	else
	{
		if (player.direction == 0)
			spSelectSprite(hase,"stand left");
		else
			spSelectSprite(hase,"stand right");
	}
	spUpdateSprite(spActiveSprite(hase),steps);
}	

void init_player()
{
	player.direction = 0;
	player.w_direction = SP_ONE/2;
	player.w_power = SP_ONE/2;
	player.hops = 0;
	int x,y;
	while (1)
	{
		x = rand()%LEVEL_WIDTH;
		y = rand()%LEVEL_HEIGHT;
		printf("Tried %i %i...",x,y);
		if (circle_is_empty(x,y,16) && gravitation_force(x,y)/32768)
			break;
		printf("NOT!\n");
	}
	printf("Fine.\n");
	player.x = x << SP_ACCURACY;
	player.y = y << SP_ACCURACY;
	player.dx = 0;
	player.dy = 0;
	posX = player.x;
	posY = player.y;
	update_player(0);
	rotation = -player.rotation;
	player.health = MAX_HEALTH;
}
