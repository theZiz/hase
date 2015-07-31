#include "level.h"
#include <stdlib.h>

void add_to_string(char* buffer,char* add)
{
	memcpy(&buffer[strlen(buffer)],add,strlen(add)+1);
}

char* create_level_string(char* buffer,int width,int height,int circles,int triangles,int quads)
{
	int t = spRand()%TEXTURE_COUNT+1;
	char temp[16];
	buffer[0] = 0;
	add_to_string(buffer,ltostr(t,temp,36));
	add_to_string(buffer," ");
	add_to_string(buffer,ltostr(width,temp,36));
	add_to_string(buffer," ");
	add_to_string(buffer,ltostr(height,temp,36));
	int i;
	for (i = 0; i < circles; i++)
	{
		int r = spRand()%(spMin(width,height) >> 3);
		int x = LEVEL_BORDER+r+spRand()%(width-2*LEVEL_BORDER-2*r);
		int y = LEVEL_BORDER+r+spRand()%(height-2*LEVEL_BORDER-2*r);
		if (spRand()%5 == 0)
			add_to_string(buffer,"-"); //negative
		add_to_string(buffer,"*"); //circle
		add_to_string(buffer,ltostr(x,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(y,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(r,temp,36));
	}
	for (i = 0; i < triangles; i++)
	{
		int r = spRand()%(spMin(width,height) >> 2);
		int x = LEVEL_BORDER+r+spRand()%(width-2*LEVEL_BORDER-2*r);
		int y = LEVEL_BORDER+r+spRand()%(height-2*LEVEL_BORDER-2*r);
		int a1 = spRand()%(SP_PI*2);
		int a2 = spRand()%(SP_PI*2);
		int a3 = spRand()%(SP_PI*2);
		Sint32 x1 = x + ( r * spCos( a1 ) >> SP_ACCURACY );
		Sint32 y1 = y + ( r * spSin( a1 ) >> SP_ACCURACY );
		Sint32 x2 = x + ( r * spCos( a2 ) >> SP_ACCURACY );
		Sint32 y2 = y + ( r * spSin( a2 ) >> SP_ACCURACY );
		Sint32 x3 = x + ( r * spCos( a3 ) >> SP_ACCURACY );
		Sint32 y3 = y + ( r * spSin( a3 ) >> SP_ACCURACY );
		if (spRand()%5 == 0)
			add_to_string(buffer,"-"); //negative
		add_to_string(buffer,"^"); //triangle
		add_to_string(buffer,ltostr(x1,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(y1,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(x2,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(y2,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(x3,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(y3,temp,36));
	}
	for (i = 0; i < quads; i++)
	{
		int r = spRand()%(spMin(width,height) >> 2);
		int angle = spRand()%(SP_PI*2);
		int x = LEVEL_BORDER+r*3/2+spRand()%(width-2*LEVEL_BORDER-3*r); //sqrt(2) ~ 2/3
		int y = LEVEL_BORDER+r*3/2+spRand()%(height-2*LEVEL_BORDER-3*r);//sqrt(2) ~ 2/3
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
		if (spRand()%5 == 0)
			add_to_string(buffer,"-"); //negative
		add_to_string(buffer,"#"); //quad
		add_to_string(buffer,ltostr(nx1,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(ny1,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(nx2,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(ny2,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(nx3,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(ny3,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(nx4,temp,36));
		add_to_string(buffer," ");
		add_to_string(buffer,ltostr(ny4,temp,36));
	}
	return buffer;
}

char* ltostr(unsigned int l,char* buffer,int base)
{
	char temp[16];
	temp[15] = 0;
	int pos = 15;
	while (l > 0 && pos > 0)
	{
		pos--;
		int rest = l % base;
		l = l / base;
		if (rest >= 0 && rest <= 9)
			temp[pos] = rest+'0';
		else
			temp[pos] = rest-10+'a';
	}
	memcpy(buffer,&temp[pos],strlen(&temp[pos])+1);
	return buffer;
}

SDL_Surface* create_level(char* level_string,int alt_width,int alt_height,int color)
{
	printf("Parse level: %s\n",level_string);
	//Lets overread the texture...
	char* mom = level_string;
	strtol(mom,&mom,36);
	//Reading the width
	int width = strtol(mom,&mom,36);
	//Reading the height
	int height = strtol(mom,&mom,36);
	
	Sint32 zoom = SP_ONE;
	int border_shift = 0;
	if (alt_width > 0 && alt_height > 0)
	{
		Sint32 xZoom = alt_width*SP_ONE / (width-2*LEVEL_BORDER);
		Sint32 yZoom = alt_height*SP_ONE / (height-2*LEVEL_BORDER);
		if (xZoom < yZoom)
			zoom = xZoom;
		else
			zoom = yZoom;
		width = (width-2*LEVEL_BORDER)*zoom >> SP_ACCURACY;
		height = (height-2*LEVEL_BORDER)*zoom >> SP_ACCURACY;
		border_shift = LEVEL_BORDER*zoom;
	}
	printf("Zoom %i, Width %i, Height %i\n",zoom,width,height);
	SDL_Surface* level = spCreateSurface(width,height);
	spSelectRenderTarget(level);
	spClearTarget(SP_ALPHA_COLOR);
	spSetAlphaTest(0);
	
	int negative = 0;
	
	while (mom[0] != 0)
	{
		Sint32 x1,y1,x2,y2,x3,y3,x4,y4;
		//Reading the kind
		switch (mom[0])
		{
			case '-':
				mom++;
				negative = 1;
				break;
			case '*': //circle
				mom++;
				x1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x2 = strtol(mom,&mom,36)*zoom >> SP_ACCURACY;
				spEllipse(x1,y1,0,x2,x2,negative?SP_ALPHA_COLOR:color);
				negative = 0;
				break;
			case '^': //triangle
				mom++;
				x1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x2 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y2 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x3 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y3 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				spTriangle(x1,y1,0,x2,y2,0,x3,y3,0,negative?SP_ALPHA_COLOR:color);
				negative = 0;
				break;
			case '#': //quads
				mom++;
				x1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y1 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x2 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y2 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x3 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y3 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				x4 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				y4 = strtol(mom,&mom,36)*zoom - border_shift >> SP_ACCURACY;
				spQuad(x1,y1,0,x2,y2,0,x3,y3,0,x4,y4,0,negative?SP_ALPHA_COLOR:color);
				negative = 0;
				break;
			default:
				mom++;
				negative = 0;
		}
	}
	spSetAlphaTest(1);
	spSelectRenderTarget(spGetWindowSurface());
	return level;
}

Uint16 level_color = 0;

Uint16 get_level_color()
{
	return level_color;
}

Uint16 border_color = 0;

Uint16 get_border_color()
{
	return border_color;
}

void texturize_level(SDL_Surface* level,char* level_string)
{
	int t = strtol(level_string,NULL,36);
	char buffer[256];
	sprintf(buffer,"./textures/texture%i.png",(t-1)%TEXTURE_COUNT+1);
	SDL_Surface* texture = spLoadSurface(buffer);
	spSelectRenderTarget(level);
	Uint16* level_pixel = spGetTargetPixel();
	SDL_LockSurface(texture);
	int texture_width = texture->pitch/texture->format->BytesPerPixel;
	Uint16* texture_pixel = (Uint16*)texture->pixels;
	int x,y;
	int r=0,g=0,b=0;
	int c = 0;
	for (x = 0; x < texture->w; x+=2)
		for (y = 0; y < texture->h; y+=2)
		{
			r += spGetRawRFromColor( texture_pixel[x+y*texture_width] );
			g += spGetRawGFromColor( texture_pixel[x+y*texture_width] );
			b += spGetRawBFromColor( texture_pixel[x+y*texture_width] );
			c++;
		}
	r /= c;
	g /= c;
	b /= c;
	level_color = (r<<11) | (g<<5) | b;
	Sint32 h = spGetHFromColor(level_color);
	Sint32 s = spGetSFromColor(level_color);
	Sint32 v = spGetVFromColor(level_color);
	border_color = spGetHSV(h,spMin(255,s*3/2),spMin(255,v*3/2));
	for (x = 0; x < level->w; x++)
		for (y = 0; y < level->h; y++)
			if (level_pixel[x+y*level->w]!= SP_ALPHA_COLOR)
				level_pixel[x+y*level->w] = texture_pixel[(x & TEXTURE_MASK) + (y & TEXTURE_MASK)*texture_width];
	SDL_UnlockSurface(texture);
	spSelectRenderTarget(spGetWindowSurface());
	spDeleteSurface(texture);
}
