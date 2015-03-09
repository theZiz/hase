#define WEAPON_X 4
#define WEAPON_Y 3

#define WEAPON_MAX 12

#define WP_BIG_BAZOOKA 0
#define WP_MID_BAZOOKA 1
#define WP_SML_BAZOOKA 2
#define WP_CLUSTER 3
#define WP_BUILD_BIG 4
#define WP_BUILD_MID 5
#define WP_BUILD_SML 6
#define WP_MINE 7
#define WP_KAIO_KEN 8
#define WP_SUPER_JUMP 9
#define WP_PREV_HARE 10
#define WP_NEXT_HARE 11

const char weapon_name[WEAPON_MAX][64] = {
	"Big carrot bazooka",
	"Middle carrot bazooka",
	"Small carrot bazooka",
	"Cluster bomb",
	"Big Build circle",
	"Middle Build circle",
	"Small Build circle",
	"Mine",
	"Kaio Ken",
	"Super jump",
	"Previous hare",
	"Next hare"
};

const char weapon_description[WEAPON_MAX][64] = {
	"Makes big holes",
	"Makes middle holes",
	"Makes small holes",
	"Makes many holes",
	"Protect yourself with much dirt",
	"Protect yourself with dirt",
	"Protect yourself with few dirt",
	"Explodes if a character is near",
	"Start the next round with 4 WP",
	"Make a very high jump",
	"Choose the previous hare",
	"Choose the next hare"
};
 
const int weapon_cost[WEAPON_MAX] = {3,2,1,3,3,2,1,3,3,1,1,1};

const int weapon_shoot[WEAPON_MAX] = {1,1,1,1,0,0,0,1,0,0,0,0};

const int weapon_explosion[WEAPON_MAX] = {32,26,20,0,0,0,0,0,0,0,0,0};

const int weapon_health_divisor[WEAPON_MAX] = {2176,2304,2432,0,0,0,0,0,0,0,0,0};

const char weapon_filename[WEAPON_MAX][64] = {
	"./data/bazooka_big.png",
	"./data/bazooka_middle.png",
	"./data/bazooka_small.png",
	"./data/cluster.png",
	"./data/build_big.png",
	"./data/build_middle.png",
	"./data/build_small.png",
	"./data/mine.png",
	"./data/kaio_ken.png",
	"./data/super_jump.png",
	"./data/prev.png",
	"./data/next.png"
};

const int weapon_pos[WEAPON_Y][WEAPON_X] = {
	{WP_BIG_BAZOOKA, WP_MID_BAZOOKA, WP_SML_BAZOOKA,   WP_CLUSTER},
	{  WP_BUILD_BIG,   WP_BUILD_MID,   WP_BUILD_SML,      WP_MINE},
	{ WP_SUPER_JUMP,    WP_KAIO_KEN,   WP_PREV_HARE, WP_NEXT_HARE}
};
 
typedef SDL_Surface *PSDL_Surface;
PSDL_Surface weapon_surface[WEAPON_MAX] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
 
void load_weapons()
{
	int i;
	for (i = 0;i<WEAPON_MAX;i++)
		weapon_surface[i] = spLoadSurface(weapon_filename[i]);
}

void delete_weapons()
{
	int i;
	for (i = 0;i<WEAPON_MAX;i++)
		spDeleteSurface(weapon_surface[i]);
}

void draw_weapons()
{
	int x,y,w,i = 0;
	for (i = 0;i<WEAPON_MAX;i++)
		w = spMax(spFontWidth(weapon_description[i],font),w);
	int factor = 24 * spGetSizeFactor() >> SP_ACCURACY;
	int h = WEAPON_Y*factor + 3*font->maxheight;
	spSetPattern8(153,60,102,195,153,60,102,195);
	spRectangle(screen->w/2,screen->h/2,0,w,h,LL_BG);
	spDeactivatePattern();
	for (x = 0;x<WEAPON_X;x++)
		for (y = 0;y<WEAPON_Y;y++)
			spRotozoomSurface((screen->w-(WEAPON_X-x-1)*factor*2+(WEAPON_X-1)*factor)/2,(screen->h-h+y*factor*2)/2+factor/2+font->maxheight,0,weapon_surface[weapon_pos[y][x]],spGetSizeFactor()/2,spGetSizeFactor()/2,0);
	spFontDraw((screen->w-w)/2,(screen->h+h)/2-font->maxheight*1,0,"[o]/[3]Choose",font);
	if (player[active_player]->activeHare)
	{
		int w_nr = weapon_pos[player[active_player]->activeHare->wp_y][player[active_player]->activeHare->wp_x];
		spFontDrawMiddle(screen->w/2,(screen->h-h)/2,0,weapon_name[w_nr],font);
		spRectangleBorder((screen->w-(WEAPON_X-player[active_player]->activeHare->wp_x-1)*factor*2+(WEAPON_X-1)*factor)/2,(screen->h-h+player[active_player]->activeHare->wp_y*factor*2)/2+factor/2+font->maxheight-1,0,factor-4,factor-4,2,2,get_border_color());
		spFontDraw((screen->w-w)/2,(screen->h+h)/2-font->maxheight*2,0,weapon_description[w_nr],font);
		char buffer[32];
		sprintf(buffer,"Cost: %i",weapon_cost[w_nr]);
		spFontDrawRight((screen->w+w)/2,(screen->h+h)/2-font->maxheight*1,0,buffer,font);
	}
}

typedef struct sBullet
{
	Sint32 x,y;
	Sint32 rotation;
	Sint32 dx,dy;
	Sint32 dr;
	Sint32 impact_state;
	Sint32 impact_counter;
	int age;
	pBulletTrace* trace;
	pBullet next;
	int kind;
	SDL_Surface* surface;
} tBullet;

#define IMPACT_TIME 30
#define OUTPACT_TIME 300
#define EXPLOSION_COLOR spGetFastRGB(255,255,255)
#define BULLET_SIZE 4
#define BULLET_SPEED_DOWN 32
#define TRACE_STEP 150

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

void addToTrace(pBulletTrace* firstTrace,Sint32 x,Sint32 y,pBullet bullet);
pBulletTrace* registerTrace(pPlayer player);

pBullet shootBullet(int x,int y,int direction,int power,Sint32 dr,pPlayer tracePlayer,SDL_Surface* surface)
{
	spSoundPlay(snd_shoot,-1,0,0,-1);
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
	bullet->age = 0;
	bullet->kind = 0;
	bullet->surface = surface;
	if (tracePlayer)
	{
		bullet->trace = registerTrace(tracePlayer);
		addToTrace(bullet->trace,bullet->x,bullet->y,bullet);
		addToTrace(bullet->trace,bullet->x,bullet->y,bullet);
	}
	else
		bullet->trace = NULL;
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
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,momBullet->surface,zoom*BULLET_SIZE/16,zoom*BULLET_SIZE/16,momBullet->rotation);
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
	int c = 0;
	int inc = (1<<gop_particles()-1)-1;
	for (x = -radius-BORDER_SIZE; x < radius+BORDER_SIZE+1; x++)
	{
		for (y = -radius-BORDER_SIZE; y < radius+BORDER_SIZE+1; y++)
		{
			int sum = x*x+y*y;
			if (sum > outer_radius)
				continue;
			if (sum <= inner_radius && pixel[X+x+(Y+y)*LEVEL_WIDTH] != SP_ALPHA_COLOR)
				if ((x & inc) == inc && (y & inc) == inc)
					c++;
		}
	}
	spParticleBunchPointer p = NULL;
	if (gop_particles() != 4)
		p = spParticleCreate(c,hare_explosion_feedback,&particles);	
	c = 0;
	int BORDER_COLOR = get_border_color();
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
					pixel[X+x+(Y+y)*LEVEL_WIDTH] = BORDER_COLOR;
			}
			else
			if (pixel[X+x+(Y+y)*LEVEL_WIDTH] != SP_ALPHA_COLOR)
			{
				if (p && (x & inc) == inc && (y & inc) == inc)
				{
					p->particle[c].x = X+x << SP_ACCURACY;
					p->particle[c].y = Y+y << SP_ACCURACY;
					p->particle[c].z = 0;
					p->particle[c].dx = ((spRand() & 131071) - SP_ONE)/4;
					p->particle[c].dy = ((spRand() & 131071) - SP_ONE)/4;
					p->particle[c].dz = 0;
					p->particle[c].data.color = BORDER_COLOR;
					p->particle[c].status = 0;
					c++;
				}
				pixel[X+x+(Y+y)*LEVEL_WIDTH] = SP_ALPHA_COLOR;
			}
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
	spSoundPlay(snd_explosion,-1,0,0,-1);
}

void negative_impact(int X,int Y,int radius)
{
	int x,y;
	int R = (radius>>GRAVITY_RESOLUTION)+GRAVITY_CIRCLE+2;
	R *= R;
	Sint32 begin = SDL_GetTicks();
	spSelectRenderTarget(level_original);
	//spSetAlphaTest(0);
	//spEllipse(X,Y,0,radius,radius,65535);
	Uint16* pixel = spGetTargetPixel();
	int inner_radius = (radius-BORDER_SIZE)*(radius-BORDER_SIZE);
	int outer_radius = radius*radius;
	int BORDER_COLOR = get_border_color();
	for (x = -radius; x < radius+1; x++)
		for (y = -radius; y < radius+1; y++)
		{
			int sum = x*x+y*y;
			if (sum > outer_radius)
				continue;
			if (sum > inner_radius)
			{
				if (pixel[X+x+(Y+y)*LEVEL_WIDTH] == SP_ALPHA_COLOR)
					pixel[X+x+(Y+y)*LEVEL_WIDTH] = BORDER_COLOR;
			}
			else
			if (pixel[X+x+(Y+y)*LEVEL_WIDTH] == SP_ALPHA_COLOR)
			{
				if ((X+x+Y+y >> 3) & 1)
					pixel[X+x+(Y+y)*LEVEL_WIDTH] = 0;
				else
					pixel[X+x+(Y+y)*LEVEL_WIDTH] = spGetFastRGB(255,255,0);
			}
		}
	Sint32 end = SDL_GetTicks();
	printf("* Recalculating level:\n");
	printf("  Drawing Ellipse: %i\n",end-begin);
	begin=end;
	posivate_gravity(X>>GRAVITY_RESOLUTION,Y>>GRAVITY_RESOLUTION,R);
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
		if (momBullet->trace)
		{
			momBullet->age++;
			if (momBullet->age%TRACE_STEP == 0)
				addToTrace(momBullet->trace,momBullet->x,momBullet->y,momBullet);
			else
			{
				(*(momBullet->trace))->x = momBullet->x;
				(*(momBullet->trace))->y = momBullet->y;
			}
		}
		momBullet->dx -= gravitation_x(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
		momBullet->dy -= gravitation_y(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
		int speed = abs(momBullet->dx)+abs(momBullet->dy);
		momBullet->rotation+=momBullet->dr*speed/BULLET_SPEED_DOWN;
		int dead = 0;
		if (momBullet->impact_state == 0 &&
			circle_is_empty(momBullet->x+momBullet->dx >> SP_ACCURACY,momBullet->y+momBullet->dy >> SP_ACCURACY,BULLET_SIZE,NULL))
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
						momBullet->impact_counter = OUTPACT_TIME;
						spClearTarget(EXPLOSION_COLOR);
						spFlip();
						bullet_impact(momBullet->x >> SP_ACCURACY,momBullet->y >> SP_ACCURACY,weapon_explosion[momBullet->kind]);
						for (j = 0; j < player_count; j++)
						{
							if (player[j]->firstHare == NULL)
								continue;
							pHare hare = player[j]->firstHare;
							if (hare)
							do
							{							
								int d = spFixedToInt(hare->x-momBullet->x)*spFixedToInt(hare->x-momBullet->x)+
										spFixedToInt(hare->y-momBullet->y)*spFixedToInt(hare->y-momBullet->y);
									d = weapon_explosion[momBullet->kind]*weapon_explosion[momBullet->kind]-d;
								if (d > 0)
								{
									hare->health -= d*MAX_HEALTH/weapon_health_divisor[momBullet->kind];
									player[j]->d_health -= d*MAX_HEALTH/weapon_health_divisor[momBullet->kind];
									player[j]->d_time = 5000;
								}
								if (hare->health <= 0)
								{
									if (hare == player[j]->activeHare ||
										hare == player[j]->setActiveHare)
									{
										player[j]->setActiveHare = hare->next;
										player[j]->activeHare = NULL;
										if (j == active_player)//Suicid!
											next_player();
									}
									hare = del_hare(hare,&(player[j]->firstHare));
									if (player[j]->firstHare == NULL)
										alive_count--;
								}
								else
									hare = hare->next;
							}
							while (hare != player[j]->firstHare);
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
			if (before)
				before->next = momBullet->next;
			else
				firstBullet = momBullet->next;
			pBullet next = momBullet->next;
			if (momBullet->trace)
				(*(momBullet->trace))->bullet = NULL;
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
				newalpha = momBullet->impact_counter*SP_ONE/OUTPACT_TIME;
				if (newalpha > alpha)
					alpha = newalpha;
				break;
		}
		momBullet = momBullet->next;
	}
	return alpha;
}
