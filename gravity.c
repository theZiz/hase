#define GRAVITY_DENSITY 32
#define GRAVITY_RESOLUTION 4
#define GRAVITY_PER_PIXEL 16
#define GRAVITY_PER_PIXEL_CORRECTION 12
#define GRAVITY_CIRCLE 32

int color_mode = 2;

typedef struct {
	Sint32 mass,x,y;
} tGravity;

SDL_Surface* gravity_surface;
tGravity* gravity;

void free_gravity()
{
	if (gravity)
		free(gravity);
}

void realloc_gravity()
{
	free_gravity();
	gravity = (tGravity*)malloc(sizeof(tGravity)*level->w*level->h>>2*GRAVITY_RESOLUTION);
}

Sint32 calc_mass(Uint16* original,int x,int y)
{
	Sint32 mass = 0;
	int a,b;
	for (a = 0; a < (1<<GRAVITY_RESOLUTION); a++)	
		for (b = 0; b < (1<<GRAVITY_RESOLUTION); b++)
		{
			if (original[(x<<GRAVITY_RESOLUTION)+a+((y<<GRAVITY_RESOLUTION)+b)*level->w] != SP_ALPHA_COLOR )
				mass+=GRAVITY_PER_PIXEL;
		}
	return mass;
}

void impact_gravity(Sint32 mass,int x,int y)
{
	int a,b;
	int start_a = x-GRAVITY_CIRCLE;
	if (start_a < 0)
		start_a = 0;
	int end_a = x+GRAVITY_CIRCLE+1;
	if (end_a > (level->w>>GRAVITY_RESOLUTION))
		end_a = (level->w>>GRAVITY_RESOLUTION);
	for (a = start_a; a < end_a; a++)
	{
		int start_b = y-GRAVITY_CIRCLE;
		if (start_b < 0)
			start_b = 0;
		int end_b = y+GRAVITY_CIRCLE+1;
		if (end_b > (level->h>>GRAVITY_RESOLUTION))
			end_b = (level->h>>GRAVITY_RESOLUTION);
		for (b = start_b; b < end_b; b++)
		if (a!=x && b!=y && (a-x)*(a-x)+(b-y)*(b-y) <= GRAVITY_CIRCLE*GRAVITY_CIRCLE)
		{
			Sint32 dx = a-x;
			Sint32 dy = b-y;
			//Sint32 sum = spFixedToInt(spSqrt(spIntToFixed(dx*dx+dy*dy+1)));
			//Sint32 sum = spFixedToInt(spSqrt(spSqrt(spIntToFixed(dx*dx+dy*dy+1))));
			Sint32 sum = dx*dx+dy*dy;
			gravity[a+b*(level->w>>GRAVITY_RESOLUTION)].x += dx*mass/sum;
			gravity[a+b*(level->w>>GRAVITY_RESOLUTION)].y += dy*mass/sum;
		}
	}
}

void negate_gravity(int mx,int my,int r)
{
	SDL_LockSurface(level_original);
	int x,y;
	int start_x = mx-r;
	int end_x = mx+r;
	int start_y = my-r;
	int end_y = my+r;
	if (start_x < 0)
		start_x = 0;
	if (start_y < 0)
		start_y = 0;
	if (end_x > (level->w >> GRAVITY_RESOLUTION))
		end_x = (level->w >> GRAVITY_RESOLUTION);
	if (end_y > (level->h >> GRAVITY_RESOLUTION))
		end_y = (level->h >> GRAVITY_RESOLUTION);	
	for (x = start_x; x < end_x; x++)
		for (y = start_y; y < end_y; y++)
			if (gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass)
			{
				int new_mass = calc_mass(level_pixel,x,y);
				int diff = new_mass-gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass;
				if (diff)
				{
					impact_gravity(diff,x,y);
					gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass = new_mass;
				}
			}
	SDL_UnlockSurface(level_original);	
}

void update_gravity()
{
	SDL_LockSurface(level_original);
	memset(gravity,0,sizeof(tGravity)*level->w*level->h>>2*GRAVITY_RESOLUTION);
	int x,y;
	for (x = 0; x < (level->w>>GRAVITY_RESOLUTION); x++)
		for (y = 0; y < (level->h>>GRAVITY_RESOLUTION); y++)
		{
			gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass = calc_mass(level_pixel,x,y);
			if (gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass)
				impact_gravity(gravity[x+y*(level->w>>GRAVITY_RESOLUTION)].mass,x,y);
		}
	SDL_UnlockSurface(level_original);	
}

void init_gravity()
{
	int x,y;
	/*char buffer[256];
	sprintf(buffer,"./levels/%s.gra",levelname);
	if (spFileExists(buffer))
	{
		loadInformation("Loading cached gravity...");
		spFilePointer file = SDL_RWFromFile(buffer,"rb");
		SDL_RWread(file,gravity,sizeof(tGravity),level->w*level->h>>2*GRAVITY_RESOLUTION);
		SDL_RWclose(file);
	}
	else*/
	{
		loadInformation("Calculating Gravity...");
		update_gravity();
		/*loadInformation("Caching Gravity...");
		spFilePointer file = SDL_RWFromFile(buffer,"wb");
		SDL_RWwrite(file,gravity,sizeof(tGravity),level->w*level->h>>2*GRAVITY_RESOLUTION);
		SDL_RWclose(file);*/
	}
	loadInformation("Drawing level...");
	spSelectRenderTarget(level);
	spClearTarget(0);
	spSetAlphaTest(1);
	/*Uint16* pixel = (Uint16*)level->pixels;
	for (x = 0; x < level->w; x++)
		for (y = 0; y < level->h; y++)
			pixel[x+y*level->w] = spGetRGB(gravitation_force(x,y)/2048,gravitation_force(x,y)/1024,gravitation_force(x,y)/512);*/
	//spSetBlending(SP_ONE*3/4);
	for (x = 0; x < (level->w>>GRAVITY_RESOLUTION); x++)
	{
		for (y = 0; y < (level->h>>GRAVITY_RESOLUTION); y++)
		{
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
			}
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
	//spSetBlending(SP_ONE);
	//spSetBlending(SP_ONE/2);
	spBlitSurface(level->w/2,level->h/2,0,level_original);
	//spSetBlending(SP_ONE);
	spSelectRenderTarget(screen);
}

void fill_gravity_surface()
{
	spSelectRenderTarget(gravity_surface);
	spClearTarget( SP_ALPHA_COLOR );
	spBindTexture( arrow );
	int x,y,s;
	Uint16 color;
	s = 1 << SP_ACCURACY+GRAVITY_RESOLUTION;
	for (x = 0; x < GRAVITY_DENSITY; x++)
	{
		int angle = (GRAVITY_DENSITY-1-x)*SP_PI*2/GRAVITY_DENSITY;
		if (color_mode == 0)
			color = spGetHSV(angle,255,255);
		for (y = 0; y < GRAVITY_DENSITY; y++)
		{
			int S;
			if (color_mode & 1)
				S = s*(GRAVITY_DENSITY-1)/GRAVITY_DENSITY/2;
			else
				S = s*(GRAVITY_DENSITY-1-y)/GRAVITY_DENSITY*3/5;
			if (color_mode)
			{
				int v = (GRAVITY_DENSITY*2-1-y)*256/(GRAVITY_DENSITY*2);
				color = spGetRGB(v,v/2,v/4);
			}
			int X = (x<<GRAVITY_RESOLUTION+1+SP_ACCURACY)+s;
			int Y = (y<<GRAVITY_RESOLUTION+1+SP_ACCURACY)+s;
			int x1 = spFixedToInt(X+spMul(spCos(angle),+S)-spMul(spSin(angle),+S));
			int y1 = spFixedToInt(Y+spMul(spSin(angle),+S)+spMul(spCos(angle),+S));
			int x2 = spFixedToInt(X+spMul(spCos(angle),+S)-spMul(spSin(angle),-S));
			int y2 = spFixedToInt(Y+spMul(spSin(angle),+S)+spMul(spCos(angle),-S));
			int x3 = spFixedToInt(X+spMul(spCos(angle),-S)-spMul(spSin(angle),-S));
			int y3 = spFixedToInt(Y+spMul(spSin(angle),-S)+spMul(spCos(angle),-S));
			int x4 = spFixedToInt(X+spMul(spCos(angle),-S)-spMul(spSin(angle),+S));
			int y4 = spFixedToInt(Y+spMul(spSin(angle),-S)+spMul(spCos(angle),+S));
			spQuad_tex( x1, y1,0,arrow->w-1,arrow->h-1,
			            x2, y2,0,arrow->w-1,         0,
			            x3, y3,0,         0,         0,
			            x4, y4,0,         0,arrow->h-1,color);
		}
	}
	spSelectRenderTarget(screen);
}
