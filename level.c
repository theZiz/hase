#define TEXTURE_SIZE 256
#define TEXTURE_MASK 255
#define TEXTURE_COUNT 8

void create_level(int circles,int triangles,int quads)
{
	spSelectRenderTarget(level_original);
	spClearTarget(SP_ALPHA_COLOR);
	int i;
	for (i = 0; i < circles; i++)
	{
		int r = rand()%(spMin(LEVEL_WIDTH,LEVEL_HEIGHT) >> 3);
		int x = LEVEL_BORDER+r+rand()%(LEVEL_WIDTH-2*LEVEL_BORDER-2*r);
		int y = LEVEL_BORDER+r+rand()%(LEVEL_HEIGHT-2*LEVEL_BORDER-2*r);
		spEllipse(x,y,0,r,r,65535);
	}
	for (i = 0; i < triangles; i++)
	{
		int r = spMin(LEVEL_WIDTH,LEVEL_HEIGHT) >> 2;
		int x3,x2,x1 = LEVEL_BORDER+r+rand()%(LEVEL_WIDTH-2*LEVEL_BORDER-2*r);
		int y3,y2,y1 = LEVEL_BORDER+r+rand()%(LEVEL_HEIGHT-2*LEVEL_BORDER-2*r);
		do
		{
			x2 = LEVEL_BORDER+r+rand()%(LEVEL_WIDTH-2*LEVEL_BORDER-2*r);
			y2 = LEVEL_BORDER+r+rand()%(LEVEL_HEIGHT-2*LEVEL_BORDER-2*r);
		}
		while ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2) > r*r);
		do
		{
			x3 = LEVEL_BORDER+r+rand()%(LEVEL_WIDTH-2*LEVEL_BORDER-2*r);
			y3 = LEVEL_BORDER+r+rand()%(LEVEL_HEIGHT-2*LEVEL_BORDER-2*r);
		}
		while ((x1-x3)*(x1-x3)+(y1-y3)*(y1-y3) > r*r);
		spTriangle(x1,y1,0,x2,y2,0,x3,y3,0,65535);
	}
	for (i = 0; i < quads; i++)
	{
		int r = rand()%(spMin(LEVEL_WIDTH,LEVEL_HEIGHT) >> 2);
		int angle = rand()%(SP_PI*2);
		int x = LEVEL_BORDER+r*3/2+rand()%(LEVEL_WIDTH-2*LEVEL_BORDER-3*r); //sqrt(2) ~ 2/3
		int y = LEVEL_BORDER+r*3/2+rand()%(LEVEL_HEIGHT-2*LEVEL_BORDER-3*r);//sqrt(2) ~ 2/3
		Sint32 x1 = -r >> 1;
		Sint32 x2 = x1;
		Sint32 x3 = r >> 1;
		Sint32 x4 = x3;
		Sint32 y1 = -r >> 1;
		Sint32 y2 = r >> 1;
		Sint32 y3 = y2;
		Sint32 y4 = y1;

		Sint32 nx1 = x + ( x1 * spCos( angle ) - y1 * spSin( angle ) >> SP_ACCURACY );
		Sint32 ny1 = y + ( y1 * spCos( angle ) + x1 * spSin( angle ) >> SP_ACCURACY );
		Sint32 nx2 = x + ( x2 * spCos( angle ) - y2 * spSin( angle ) >> SP_ACCURACY );
		Sint32 ny2 = y + ( y2 * spCos( angle ) + x2 * spSin( angle ) >> SP_ACCURACY );
		Sint32 nx3 = x + ( x3 * spCos( angle ) - y3 * spSin( angle ) >> SP_ACCURACY );
		Sint32 ny3 = y + ( y3 * spCos( angle ) + x3 * spSin( angle ) >> SP_ACCURACY );
		Sint32 nx4 = x + ( x4 * spCos( angle ) - y4 * spSin( angle ) >> SP_ACCURACY );
		Sint32 ny4 = y + ( y4 * spCos( angle ) + x4 * spSin( angle ) >> SP_ACCURACY );

		spQuad(nx1,ny1,0,nx2,ny2,0,nx3,ny3,0,nx4,ny4,0,65535);
	}
	spSelectRenderTarget(screen);
}

void texturize_level()
{
	int t = rand()%TEXTURE_COUNT+1;
	char buffer[256];
	sprintf(buffer,"./textures/texture%i.png",t);
	SDL_Surface* texture = spLoadSurface(buffer);
	spSelectRenderTarget(level_original);
	Uint16* level_pixel = spGetTargetPixel();
	SDL_LockSurface(texture);
	Uint16* texture_pixel = (Uint16*)texture->pixels;
	int x,y;
	for (x = 0; x < LEVEL_WIDTH; x++)
		for (y = 0; y < LEVEL_HEIGHT; y++)
			if (level_pixel[x+y*LEVEL_WIDTH]!= SP_ALPHA_COLOR)
				level_pixel[x+y*LEVEL_WIDTH] = texture_pixel[(x & TEXTURE_MASK) + (y & TEXTURE_MASK)*TEXTURE_SIZE];
	SDL_UnlockSurface(texture);
	spSelectRenderTarget(screen);
	spDeleteSurface(texture);
}
