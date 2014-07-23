typedef struct sBullet
{
	Sint32 x,y;
	Sint32 rotation;
	Sint32 dx,dy;
	Sint32 dr;
	Sint32 impact_state;
	Sint32 impact_counter;
	pBullet next;
} tBullet;

#define IMPACT_TIME 300
#define EXPLOSION_COLOR spGetFastRGB(255,255,255)
#define BULLET_SIZE 4
#define BULLET_SPEED_DOWN 32

pBullet firstBullet = NULL;

void deleteAllBullets()
{
	while (firstBullet)
	{
		pBullet next = firstBullet->next;
		free(firstBullet);
		firstBullet = next;
	}
}

pBullet shootBullet(int x,int y,int direction,int power,Sint32 dr)
{
	pBullet bullet = (pBullet)malloc(sizeof(tBullet));
	bullet->next = firstBullet;
	firstBullet = bullet;
	bullet->x = x+(10+BULLET_SIZE)*spCos(direction);
	bullet->y = y+(10+BULLET_SIZE)*spSin(direction);
	bullet->dr = dr;
	bullet->impact_state = 0;
	bullet->rotation = spRand()%(2*SP_PI);
	bullet->dx = spMul(spCos(direction),power);
	bullet->dy = spMul(spSin(direction),power);
	return bullet;
}

void drawBullets()
{
	pBullet momBullet = firstBullet;
	while (momBullet)
	{
		if (momBullet->impact_state < 1)
		{
			Sint32 ox = spMul(momBullet->x-posX,zoom);
			Sint32 oy = spMul(momBullet->y-posY,zoom);
			Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
			Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,bullet,zoom*BULLET_SIZE/16,zoom*BULLET_SIZE/16,momBullet->rotation);
		}
		momBullet = momBullet->next;
	}
}

void bullet_impact(int X,int Y,int radius)
{
	int x,y;
	int R = (radius>>GRAVITY_RESOLUTION)+GRAVITY_CIRCLE+2;
	R *= R;
	Sint32 begin = SDL_GetTicks();
	spSelectRenderTarget(level_original);
	//spSetAlphaTest(0);
	//spEllipse(X,Y,0,radius,radius,SP_ALPHA_COLOR);
	Uint16* pixel = spGetTargetPixel();
	int inner_radius = radius*radius;
	int outer_radius = (radius+BORDER_SIZE)*(radius+BORDER_SIZE);
	for (x = -radius-BORDER_SIZE; x < radius+BORDER_SIZE+1; x++)
	{
		for (y = -radius-BORDER_SIZE; y < radius+BORDER_SIZE+1; y++)
		{
			int sum = x*x+y*y;
			if (sum > outer_radius)
				continue;
			if (sum > inner_radius)
			{
				if (pixel[X+x+(Y+y)*LEVEL_WIDTH] != SP_ALPHA_COLOR)
					pixel[X+x+(Y+y)*LEVEL_WIDTH] = spGetFastRGB(255,200,0);
			}
			else
				pixel[X+x+(Y+y)*LEVEL_WIDTH] = SP_ALPHA_COLOR;
		}
	}	
	Sint32 end = SDL_GetTicks();
	printf("* Recalculating level:\n");
	printf("  Drawing Ellipse: %i\n",end-begin);
	begin=end;
	negate_gravity(X>>GRAVITY_RESOLUTION,Y>>GRAVITY_RESOLUTION,R);
	end = SDL_GetTicks();
	printf("  Update Gravity: %i\n",end-begin);
	begin=end;
	spSelectRenderTarget(level);
	spSetAlphaTest(1);
	/*Uint16* pixel = (Uint16*)level->pixels;
	for (x = 0; x < LEVEL_WIDTH; x++)
		for (y = 0; y < LEVEL_HEIGHT; y++)
			pixel[x+y*LEVEL_WIDTH] = spGetRGB(gravitation_force(x,y)/2048,gravitation_force(x,y)/1024,gravitation_force(x,y)/512);*/
	spEllipse(X,Y,0,radius+(GRAVITY_CIRCLE<<GRAVITY_RESOLUTION),radius+(GRAVITY_CIRCLE<<GRAVITY_RESOLUTION),0);
	end = SDL_GetTicks();
	printf("  Drawing Ellipse: %i\n",end-begin);
	begin=end;
	//spSetBlending(SP_ONE*3/4);
	int start_x = (X - radius >> GRAVITY_RESOLUTION) - 2 - GRAVITY_CIRCLE;
	int end_x = (X + radius >> GRAVITY_RESOLUTION) + 2 + GRAVITY_CIRCLE;
	int start_y = (Y - radius >> GRAVITY_RESOLUTION) - 2 - GRAVITY_CIRCLE;
	int end_y = (Y + radius >> GRAVITY_RESOLUTION) + 2 + GRAVITY_CIRCLE;
	if (start_x < 0)
		start_x = 0;
	if (start_y < 0)
		start_y = 0;
	if (end_x > (LEVEL_WIDTH >> GRAVITY_RESOLUTION))
		end_x = (LEVEL_WIDTH >> GRAVITY_RESOLUTION);
	if (end_y > (LEVEL_HEIGHT >> GRAVITY_RESOLUTION))
		end_y = (LEVEL_HEIGHT >> GRAVITY_RESOLUTION);	
	int max_mass = GRAVITY_PER_PIXEL << 2*GRAVITY_RESOLUTION;
	for (x = start_x; x < end_x; x++)
	{
		for (y = start_y; y < end_y; y++)
		{
			if (gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].mass == max_mass)
				continue;
			if ((x-(X >> GRAVITY_RESOLUTION))*(x-(X >> GRAVITY_RESOLUTION))+(y-(Y >> GRAVITY_RESOLUTION))*(y-(Y >> GRAVITY_RESOLUTION)) > R)
				continue;
			Sint32 force = spSqrt(spMul(gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x,
			                      gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x)+
			                      spMul(gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y,
			                      gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y));			                      
			int f = GRAVITY_DENSITY-1-force / GRAVITY_PER_PIXEL_CORRECTION / (16384/GRAVITY_DENSITY);
			if (f < 0)
				f = 0;
			if (f == 31 && force != 0)
				f = 31;
			Sint32 angle = 0;
			if (force)
			{
				Sint32 ac = spDiv(gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].y,force);
				if (ac < -SP_ONE)
					ac = -SP_ONE;
				if (ac > SP_ONE)
					ac = SP_ONE;
				angle = spAcos(ac)*(GRAVITY_DENSITY/2)/SP_PI;
				if (gravity[x+y*(LEVEL_WIDTH>>GRAVITY_RESOLUTION)].x <= 0)
					angle = GRAVITY_DENSITY-1-angle;
				if (angle > GRAVITY_DENSITY-1)
					angle = GRAVITY_DENSITY-1;
				if (angle < 0)
					angle = 0;
				spBlitSurfacePart(x<<GRAVITY_RESOLUTION,y<<GRAVITY_RESOLUTION,0,
								  gravity_surface,angle<<GRAVITY_RESOLUTION+1,f<<GRAVITY_RESOLUTION+1,1<<GRAVITY_RESOLUTION+1,1<<GRAVITY_RESOLUTION+1);
			}
		}
	}
	//spSetBlending(SP_ONE);
	//spSetBlending(SP_ONE/2);
	end = SDL_GetTicks();
	printf("  Drawing Arrows: %i\n",end-begin);
	begin=end;
	spSetHorizontalOrigin(SP_LEFT);
	spSetVerticalOrigin(SP_TOP);
	spSetAlphaTest(1);
	spBlitSurfacePart(X-radius-(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION),
	                  Y-radius-(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION),
	                  0,level_original,
	                  X-radius-(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION),
	                  Y-radius-(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION),
	                  2*(radius+(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION)),
	                  2*(radius+(GRAVITY_CIRCLE + 2 << GRAVITY_RESOLUTION)));
	spSetHorizontalOrigin(SP_CENTER);
	spSetVerticalOrigin(SP_CENTER);
	//spBlitSurface(LEVEL_WIDTH/2,LEVEL_HEIGHT/2,0,level_original);
	end = SDL_GetTicks();
	printf("  Blitting: %i\n",end-begin);
	begin=end;
	//spSetBlending(SP_ONE);
	spSelectRenderTarget(screen);
}

int updateBullets()
{
	pBullet momBullet = firstBullet;
	pBullet before = NULL;
	while (momBullet)
	{
		momBullet->dx -= gravitation_x(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
		momBullet->dy -= gravitation_y(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
		int speed = abs(momBullet->dx)+abs(momBullet->dy);
		momBullet->rotation+=momBullet->dr*speed/BULLET_SPEED_DOWN;
		int dead = 0;
		if (momBullet->impact_state == 0 &&
			circle_is_empty(momBullet->x+momBullet->dx >> SP_ACCURACY,momBullet->y+momBullet->dy >> SP_ACCURACY,BULLET_SIZE,-1))
		{
			momBullet->x += momBullet->dx;
			momBullet->y += momBullet->dy;
		}
		else
		{
			int j;
			switch (momBullet->impact_state)
			{
				case 0:
					momBullet->impact_state = 1;
					momBullet->impact_counter = IMPACT_TIME;
					break;
				case 1:
					momBullet->impact_counter--;
					if (momBullet->impact_counter == 0)
					{
						momBullet->impact_state = 2;
						momBullet->impact_counter = IMPACT_TIME;
						spClearTarget(EXPLOSION_COLOR);
						spFlip();
						bullet_impact(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY,32);
						for (j = 0; j < player_count; j++)
						{
							if (player[j]->health == 0)
								continue;
							int d = spFixedToInt(player[j]->x-momBullet->x)*spFixedToInt(player[j]->x-momBullet->x)+
									spFixedToInt(player[j]->y-momBullet->y)*spFixedToInt(player[j]->y-momBullet->y);
								d = 1024-d;
							if (d > 0)
								player[j]->health -= d*MAX_HEALTH/2048;
							if (player[j]->health <= 0)
							{
								player[j]->health = 0;
								alive_count--;
							}
						}
						spResetLoop();
						return 2;
					}
					break;
				case 2:
					momBullet->impact_counter--;
					if (momBullet->impact_counter == 0)
					{
						dead = 1;
						if (alive_count < 2)
							return 1;
					}
					break;
			}
		}
		if (dead || momBullet->x < 0 || momBullet->y < 0 || spFixedToInt(momBullet->x) >= LEVEL_WIDTH || spFixedToInt(momBullet->y) >= LEVEL_HEIGHT)
		{
			if (momBullet == player[active_player]->bullet)
				next_player();
			if (before)
				before->next = momBullet->next;
			else
				firstBullet = momBullet->next;
			pBullet next = momBullet->next;
			free(momBullet);
			momBullet = next;
		}
		else
		{
			before = momBullet;
			momBullet = momBullet->next;
		}
	}
	return 0;
}

Sint32 bullet_alpha()
{
	Sint32 alpha = 0;
	Sint32 newalpha;
	pBullet momBullet = firstBullet;
	while (momBullet)
	{
		switch (momBullet->impact_state)
		{
			case 1:
				newalpha = SP_ONE-momBullet->impact_counter*SP_ONE/IMPACT_TIME;
				if (newalpha > alpha)
					alpha = newalpha;
				break;
			case 2:
				newalpha = momBullet->impact_counter*SP_ONE/IMPACT_TIME;
				if (newalpha > alpha)
					alpha = newalpha;
				break;
		}
		momBullet = momBullet->next;
	}
	return alpha;
}
