spSpriteCollectionPointer hase;
struct
{
	int direction;
	Sint32 x,y;
	Sint32 rotation;
} player;

int circle_is_empty(int x, int y, int r)
{
	int a,b;
	int start_a = x-r;
	if (start_a < 0)
		start_a = 0;
	int end_a = x+r+1;
	if (end_a > level->w)
		end_a = level->w;
	for (a = start_a; a < end_a; a++)
	{
		int start_b = y-r;
		if (start_b < 0)
			start_b = 0;
		int end_b = y+r+1;
		if (end_b > level->h)
			end_b = level->h;
		for (b = start_b; b < end_b; b++)
		if (a!=x && b!=y && (a-x)*(a-x)+(b-y)*(b-y) <= r*r)
		{
			if (level_pixel[a+b*level->w] != SP_ALPHA_COLOR)
				return 0;
		}
	}
	
	return 1;
}

Sint32 gravitation_force(int x,int y)
{
	int gx = x >> GRAVITY_RESOLUTION;
	int gy = y >> GRAVITY_RESOLUTION;
	if (gx < 0 || gy < 0 || gx >= (level->w >> GRAVITY_RESOLUTION) || gy >= (level->h >> GRAVITY_RESOLUTION))
		return 0;
	Sint32 force = spSqrt(spMul(gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].x,
						        gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].x)+
						  spMul(gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].y,
						        gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].y));
	return force;
}


void update_player()
{
	player.rotation = 0;
	int gx = player.x >> SP_ACCURACY + GRAVITY_RESOLUTION;
	int gy = player.y >> SP_ACCURACY + GRAVITY_RESOLUTION;
	Sint32 force = spSqrt(spMul(-gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].x,
						        -gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].x)+
						  spMul(-gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].y,
						        -gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].y));			                      
	if (force)
	{
		Sint32 ac = spDiv(-gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].y,force);
		if (ac < -SP_ONE)
			ac = -SP_ONE;
		if (ac > SP_ONE)
			ac = SP_ONE;
		player.rotation = -spAcos(ac);
		if (-gravity[gx+gy*(level->w>>GRAVITY_RESOLUTION)].x <= 0)
			player.rotation = 2*SP_PI-player.rotation;
	}
}

void init_player()
{
	player.direction = 0;
	int x,y;
	while (1)
	{
		x = rand()%level->w;
		y = rand()%level->h;
		printf("Tried %i %i...",x,y);
		if (circle_is_empty(x,y,16) && gravitation_force(x,y))
			break;
		printf("NOT!\n");
	}
	printf("Fine.\n");
	player.x = x << SP_ACCURACY;
	player.y = y << SP_ACCURACY;
	posX = player.x;
	posY = player.y;
	update_player();
}
