#define TRACE_LENGTH 200
#define TRACE_STEP 100
#define TRACE_UPDATE 5
int trace_count = TRACE_UPDATE-1;
typedef struct sTrace
{
	Sint32 x,y;
} tTrace;

tTrace trace[TRACE_LENGTH];
int traceLength;


void updateTrace()
{
	if (spGetFPS() < 30)
	{
		trace_count++;
		if (trace_count >= TRACE_UPDATE)
			trace_count = 0;
		else
			return;
	}
	int i,j;
	trace[0].x = player.x;
	trace[0].y = player.y;
	traceLength = 0;
	Sint32 dx = spMul(spCos(player.w_direction+player.rotation+SP_PI),player.w_power/2);
	Sint32 dy = spMul(spSin(player.w_direction+player.rotation+SP_PI),player.w_power/2);
	Sint32 x = player.x;
	Sint32 y = player.y;
	for (j = 1; j < TRACE_LENGTH; j++)
	{
		int dead = 0;
		for (i = 0; i < TRACE_STEP; i++)
		{
			dx -= gravitation_x(x >> SP_ACCURACY,y >> SP_ACCURACY)/8192;
			dy -= gravitation_y(x >> SP_ACCURACY,y >> SP_ACCURACY)/8192;
			if (circle_is_empty(x+dx >> SP_ACCURACY,y+dy >> SP_ACCURACY,2) && x >= 0 && y >= 0 && spFixedToInt(x) < level->w && spFixedToInt(y) < level->h)
			{
				x += dx;
				y += dy;
			}
			else
			{
				dead = 1;
				break;
			}
		}
		trace[j].x = x;
		trace[j].y = y;
		if (dead)
		{
			j++;
			break;
		}
	}
	traceLength = j;
}

void drawTrace()
{
	Sint32 ox = spMul(trace[0].x-posX,zoom);
	Sint32 oy = spMul(trace[0].y-posY,zoom);
	Sint32 x0 = screen->w/2+(spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY);
	Sint32 y0 = screen->h/2+(spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY);
	int j;
	for (j = 1; j < traceLength; j++)
	{
		ox = spMul(trace[j].x-posX,zoom);
		oy = spMul(trace[j].y-posY,zoom);
		Sint32 x1 = screen->w/2+(spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY);
		Sint32 y1 = screen->h/2+(spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY);
		spLine(x0,y0,0,x1,y1,0,spGetFastRGB(255,0,0));
		x0 = x1;
		y0 = y1;
	}
}
