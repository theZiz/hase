const char item_filename[ITEMS_COUNT+1][64] = {
	"./data/health.png",
	"./data/power.png",
	"./data/mine.png",
	"./data/mine_beep.png"
};

PSDL_Surface item_surface[ITEMS_COUNT+1] = {NULL,NULL,NULL};

pItem items_drop(int kind,Sint32 x,Sint32 y)
{
	int tries = 0;
	if (x == -1 && y == -1)
	while (1)
	{
		x = spRand()%LEVEL_WIDTH;
		y = spRand()%LEVEL_HEIGHT;
		if (circle_is_empty(x<<SP_ACCURACY,y<<SP_ACCURACY,16,NULL,1) && gravitation_force(x,y)/32768)
			break;
		if (++tries > 1000)
			return NULL;
	}
	pItem item = (pItem)malloc(sizeof(tItem));
	item->x = x << SP_ACCURACY;
	item->y = y << SP_ACCURACY;
	item->dx = 0;
	item->dy = 0;
	item->kind = kind;
	item->rotation = 0;
	item->seen = 1500;
	item->beep = 0;
	item->next = firstItem;
	firstItem = item;
	return item;
}

void items_init(pGame game)
{
	int i;
	for (i = 0; i <= ITEMS_COUNT; i++)
		item_surface[i] = spLoadSurface(item_filename[i]);
}

void items_calc()
{
	pItem item = firstItem;
	pItem before = NULL;
	while (item)
	{
		if (item->seen)
			item->seen--;
		Sint32 force = gravitation_force(spFixedToInt(item->x),spFixedToInt(item->y));
		if (force)
		{
			Sint32 ac = spDiv(-gravitation_y(spFixedToInt(item->x),spFixedToInt(item->y)),force);
			if (ac < -SP_ONE)
				ac = -SP_ONE;
			if (ac > SP_ONE)
				ac = SP_ONE;
			item->rotation = -spAcos(ac);
			if (-gravitation_x(spFixedToInt(item->x),spFixedToInt(item->y)) <= 0)
				item->rotation = 2*SP_PI-item->rotation;
			while (item->rotation < 0)
				item->rotation += 2*SP_PI;
			while (item->rotation >= 2*SP_PI)
				item->rotation -= 2*SP_PI;
		}
		item->dx -= gravitation_x(spFixedToInt(item->x),spFixedToInt(item->y)) >> PHYSIC_IMPACT;
		item->dy -= gravitation_y(spFixedToInt(item->x),spFixedToInt(item->y)) >> PHYSIC_IMPACT;
		if (circle_is_empty(item->x+item->dx,item->y+item->dy,PLAYER_RADIUS,NULL,1))
		{
			item->x += item->dx;
			item->y += item->dy;
		}
		else
		if (item->dx || item->dy)
		{
			//bounce (copy & paste from particle feedback function)!
			Sint32 speed = vector_length_approx(item->dx,item->dy);
			if (speed <= (SP_ONE >> 4))
			{
				item->dx = 0;
				item->dy = 0;
			}
			else
			{
				Sint32 ax = spDiv(item->dx,speed);
				Sint32 ay = spDiv(item->dy,speed);
				Sint32 ex = gravitation_x(item->x >> SP_ACCURACY,item->y >> SP_ACCURACY);
				Sint32 ey = gravitation_y(item->x >> SP_ACCURACY,item->y >> SP_ACCURACY);
				Sint32 len = vector_length_approx(ex,ey);
				if (len == 0)
				{
					item->dx = 0;
					item->dy = 0;
				}
				else
				{
					ex = spDiv(ex,len);
					ey = spDiv(ey,len);
					Sint32 p = -2*(spMul(ex,ax)+spMul(ey,ay));
					item->dx = spMul(p,ex)+ax;
					item->dy = spMul(p,ey)+ay;
					item->dx = spMul(item->dx,speed*3/4);
					item->dy = spMul(item->dy,speed*3/4);
				}
			}
		}
		int j,dead = 0;
		if (item->beep)
		{
			item->beep--;
			if (item->beep == 0)
			{
				pBullet bullet = shootBullet(0,0,0,0,0,NULL,NULL,WP_MINE_BOMB,0);
				bullet->x = item->x;
				bullet->y = item->y;
				dead = 1;
			}
		}
		if (dead == 0)
			for (j = 0; j < player_count; j++)
			{
				pHare hare = player[j]->firstHare;
				if (hare)
				do
				{
					int d = spFixedToInt(hare->x-item->x)*spFixedToInt(hare->x-item->x)+
							spFixedToInt(hare->y-item->y)*spFixedToInt(hare->y-item->y);
					switch (item->kind)
					{
						case 0: //health
							if (d <= PLAYER_RADIUS*PLAYER_RADIUS*4)
							{
								hare->health += 50;
								dead = 1;
							}
							break;
						case 1: //power
							if (d <= PLAYER_RADIUS*PLAYER_RADIUS*4)
							{
								if (player[j]->computer == 0)
									player[j]->weapon_points++;
								dead = 1;
							}
							break;
						case 2: //mine
							if (item->beep == 0 && d <= (PLAYER_RADIUS+32)*(PLAYER_RADIUS+32))
							{
								item->beep = 5000;
								if (speed == 1)
									spSoundPlay(snd_beep,-1,0,0,-1);
							}
							break;
					}
					if (dead)
						break;
					hare = hare->next;
				}
				while (hare != player[j]->firstHare);
				if (dead)
					break;
			}
		pItem next = item->next;
		if (dead || item->x < 0 || item->y < 0 || spFixedToInt(item->x) >= LEVEL_WIDTH || spFixedToInt(item->y) >= LEVEL_HEIGHT)
		{
			if (before)
				before->next = item->next;
			else
				firstItem = item->next;
			if (item == dropItem)
				dropItem = NULL;
			free(item);
		}
		else
			before = item;
		item = next;
	}
	
}

void items_draw()
{
	pItem item = firstItem;
	while (item)
	{
		Sint32 ox = spMul(item->x-posX,zoom);
		Sint32 oy = spMul(item->y-posY,zoom);
		Sint32	x = spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY;
		Sint32	y = spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY;
		if (item->kind == 2 && item->beep/500%2)
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,item_surface[ITEMS_COUNT],zoom/2,zoom/2,rotation+item->rotation);
		else
			spRotozoomSurface(screen->w/2+x,screen->h/2+y,0,item_surface[item->kind],zoom/2,zoom/2,rotation+item->rotation);
		item = item->next;
	}
}

void items_quit()
{
	int i;
	for (i = 0; i <= ITEMS_COUNT; i++)
		spDeleteSurface(item_surface[i]);
	while (firstItem)
	{
		pItem next = firstItem->next;
		free(firstItem);
		firstItem = next;
	}	
}

