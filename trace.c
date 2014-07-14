#define TRACE_LENGTH 20
#define TRACE_STEP 100
#define TRACE_UPDATE 5
int trace_count = TRACE_UPDATE-1;
typedef struct sTrace
{
	Sint32 x,y;
} tTrace;

tTrace trace[TRACE_LENGTH];
int traceLength;

void lastPoint(int* x,int* y,int direction,int power)
{
	int i,j;
	Sint32 dx = spMul(spCos(direction),power);
	Sint32 dy = spMul(spSin(direction),power);
	(*x) += (10+BULLET_SIZE)*spCos(direction);
	(*y) += (10+BULLET_SIZE)*spSin(direction);
	for (j = 1; j < TRACE_LENGTH; j++)
		for (i = 0; i < TRACE_STEP; i++)
		{
			dx -= gravitation_x((*x) >> SP_ACCURACY,(*y) >> SP_ACCURACY) >> PHYSIC_IMPACT;
			dy -= gravitation_y((*x) >> SP_ACCURACY,(*y) >> SP_ACCURACY) >> PHYSIC_IMPACT;
			if (circle_is_empty((*x)+dx >> SP_ACCURACY,(*y)+dy >> SP_ACCURACY,BULLET_SIZE,-1) &&
			    (*x) >= 0 && (*y) >= 0 &&
			    spFixedToInt((*x)) < LEVEL_WIDTH && spFixedToInt((*y)) < LEVEL_HEIGHT)
			{
				(*x) += dx;
				(*y) += dy;
			}
			else
				return;
		}
}

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
	traceLength = 0;
	Sint32 dx = spMul(spCos(player[active_player]->w_direction+player[active_player]->rotation+SP_PI),player[active_player]->w_power/2);
	Sint32 dy = spMul(spSin(player[active_player]->w_direction+player[active_player]->rotation+SP_PI),player[active_player]->w_power/2);
	Sint32 x = player[active_player]->x+(10+BULLET_SIZE)*spCos(player[active_player]->w_direction+player[active_player]->rotation+SP_PI);
	Sint32 y = player[active_player]->y+(10+BULLET_SIZE)*spSin(player[active_player]->w_direction+player[active_player]->rotation+SP_PI);
	trace[0].x = x;
	trace[0].y = y;
	for (j = 1; j < TRACE_LENGTH; j++)
	{
		int dead = 0;
		for (i = 0; i < TRACE_STEP; i++)
		{
			dx -= gravitation_x(x >> SP_ACCURACY,y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			dy -= gravitation_y(x >> SP_ACCURACY,y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			if (circle_is_empty(x+dx >> SP_ACCURACY,y+dy >> SP_ACCURACY,BULLET_SIZE,-1) &&
			    x >= 0 && y >= 0 &&
			    spFixedToInt(x) < LEVEL_WIDTH && spFixedToInt(y) < LEVEL_HEIGHT)
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
		spLine(x0,y0,0,x1,y1,0,spGetFastRGB(255,160,0));
		x0 = x1;
		y0 = y1;
	}
}
