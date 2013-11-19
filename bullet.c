typedef struct sBullet *pBullet;
typedef struct sBullet
{
	Sint32 x,y;
	Sint32 rotation;
	Sint32 dr;
	Sint32 dx,dy;
	Sint32 impact_state;
	Sint32 impact_counter;
	pBullet next;
} tBullet;

#define IMPACT_TIME 100
#define EXPLOSION_COLOR spGetFastRGB(255,255,255)

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

void shootBullet(int x,int y,int direction,int power)
{
	pBullet bullet = (pBullet)malloc(sizeof(tBullet));
	bullet->next = firstBullet;
	firstBullet = bullet;
	bullet->x = x;
	bullet->y = y;
	bullet->impact_state = 0;
	bullet->rotation = rand()/(2*SP_PI);
	bullet->dr = rand()/(2*SP_PI/100);
	bullet->dx = spMul(spCos(direction),power);
	bullet->dy = spMul(spSin(direction),power);
}

void drawBullets()
{
	pBullet momBullet = firstBullet;
	while (momBullet)
	{
		if (momBullet->impact_state < 2)
		{
			Sint32 x,y;
			if (map_follows)
			{
				Sint32 ox = spMul(momBullet->x-posX,zoom);
				Sint32 oy = spMul(momBullet->y-posY,zoom);
				x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
				y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
			}
			else
			{
				x = spMul(momBullet->x-posX,zoom) >> SP_ACCURACY;
				y = spMul(momBullet->y-posY,zoom) >> SP_ACCURACY;
			}
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,bullet,zoom/8,zoom/8,momBullet->rotation);
		}
		momBullet = momBullet->next;
	}
}

void bullet_impact(int X,int Y,int radius)
{
	int R = (radius>>GRAVITY_RESOLUTION)+GRAVITY_CIRCLE+2;
	R *= R;
	Sint32 begin = SDL_GetTicks();
	spSelectRenderTarget(level_original);
	spSetAlphaTest(0);
	spEllipse(X,Y,0,radius,radius,SP_ALPHA_COLOR);
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
	for (x = 0; x < level->w; x++)
		for (y = 0; y < level->h; y++)
			pixel[x+y*level->w] = spGetRGB(gravitation_force(x,y)/2048,gravitation_force(x,y)/1024,gravitation_force(x,y)/512);*/
	spEllipse(X,Y,0,radius+(GRAVITY_CIRCLE<<GRAVITY_RESOLUTION),radius+(GRAVITY_CIRCLE<<GRAVITY_RESOLUTION),0);
	end = SDL_GetTicks();
	printf("  Drawing Ellipse: %i\n",end-begin);
	begin=end;
	//spSetBlending(SP_ONE*3/4);
	int x,y;
	int start_x = (X - radius >> GRAVITY_RESOLUTION) - 2 - GRAVITY_CIRCLE;
	int end_x = (X + radius >> GRAVITY_RESOLUTION) + 2 + GRAVITY_CIRCLE;
	int start_y = (Y - radius >> GRAVITY_RESOLUTION) - 2 - GRAVITY_CIRCLE;
	int end_y = (Y + radius >> GRAVITY_RESOLUTION) + 2 + GRAVITY_CIRCLE;
	if (start_x < 0)
		start_x = 0;
	if (start_y < 0)
		start_y = 0;
	if (end_x > (level->w >> GRAVITY_RESOLUTION))
		end_x = (level->w >> GRAVITY_RESOLUTION);
	if (end_y > (level->h >> GRAVITY_RESOLUTION))
		end_y = (level->h >> GRAVITY_RESOLUTION);	
	int max_mass = GRAVITY_PER_PIXEL << 2*GRAVITY_RESOLUTION;
	for (x = start_x; x < end_x; x++)
	{
		for (y = start_y; y < end_y; y++)
		{
			if (gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass == max_mass)
				continue;
			if ((x-(X >> GRAVITY_RESOLUTION))*(x-(X >> GRAVITY_RESOLUTION))+(y-(Y >> GRAVITY_RESOLUTION))*(y-(Y >> GRAVITY_RESOLUTION)) > R)
				continue;
			Sint32 force = spSqrt(spMul(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].x,
			                      gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].x)+
			                      spMul(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y,
			                      gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y));			                      
			int f = GRAVITY_DENSITY-1-force / GRAVITY_PER_PIXEL_CORRECTION / (16384/GRAVITY_DENSITY);
			if (f < 0)
				f = 0;
			if (f == 31 && force != 0)
				f = 31;
			Sint32 angle = 0;
			if (force)
			{
				Sint32 ac = spDiv(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].y,force);
				if (ac < -SP_ONE)
					ac = -SP_ONE;
				if (ac > SP_ONE)
					ac = SP_ONE;
				angle = spAcos(ac)*(GRAVITY_DENSITY/2)/SP_PI;
				if (gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].x <= 0)
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
	//spBlitSurface(level->w/2,level->h/2,0,level_original);
	end = SDL_GetTicks();
	printf("  Blitting: %i\n",end-begin);
	begin=end;
	//spSetBlending(SP_ONE);
	spSelectRenderTarget(screen);
}

void updateBullets(int steps)
{
	int i;
	for (i = 0; i < steps; i++)
	{
		pBullet momBullet = firstBullet;
		pBullet before = NULL;
		while (momBullet)
		{
			momBullet->dx -= gravitation_x(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY)/8192;
			momBullet->dy -= gravitation_y(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY)/8192;
			int dead = 0;
			if (momBullet->impact_state == 0 && circle_is_empty(momBullet->x+momBullet->dx >> SP_ACCURACY,momBullet->y+momBullet->dy >> SP_ACCURACY,2))
			{
				momBullet->x += momBullet->dx;
				momBullet->y += momBullet->dy;
			}
			else
			{
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
							spResetLoop();
							return;
						}
						break;
					case 2:
						momBullet->impact_counter--;
						if (momBullet->impact_counter == 0)
							dead = 1;
						break;
				}
			}
			if (dead || momBullet->x < 0 || momBullet->y < 0 || spFixedToInt(momBullet->x) >= level->w || spFixedToInt(momBullet->y) >= level->h)
			{
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
	}
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
				newalpha = SP_ONE-momBullet->impact_counter*SP_ONE/100;
				if (newalpha > alpha)
					alpha = newalpha;
				break;
			case 2:
				newalpha = momBullet->impact_counter*SP_ONE/100;
				if (newalpha > alpha)
					alpha = newalpha;
				break;
		}
		momBullet = momBullet->next;
	}
	return alpha;
}
