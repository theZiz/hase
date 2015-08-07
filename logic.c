int do_physics()
{
	int j;
	for (j = 0; j < player_count; j++)
	{
		pHare hare = player[j]->firstHare;
		if (hare)
		do
		{	
			memset(hare->circle_checkpoint_hit,0,sizeof(int)*CIRCLE_CHECKPOINTS);
			memset(hare->circle_checkpoint_hare,0,sizeof(pHare)*CIRCLE_CHECKPOINTS);
			hare = hare->next;
		}
		while (hare != player[j]->firstHare);
	}
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->firstHare == NULL)
			continue;
		if (player[j]->d_time > 0)
			player[j]->d_time--;
		if (player[j]->d_time == 0)
			player[j]->d_health = 0;
		pHare hare = player[j]->firstHare;
		if (hare)
		do
		{							
			hare->dx -= gravitation_x(spFixedToInt(hare->x),spFixedToInt(hare->y)) >> PHYSIC_IMPACT;
			hare->dy -= gravitation_y(spFixedToInt(hare->x),spFixedToInt(hare->y)) >> PHYSIC_IMPACT;
			if (circle_is_empty(hare->x+hare->dx,hare->y+hare->dy,PLAYER_RADIUS,hare,1))
			{
				hare->x += hare->dx;
				hare->y += hare->dy;
			}
			else
			if (hare->dx || hare->dy)
			{
				hare->dx = 0;
				hare->dy = 0;
				int k;
				for (k = 0; k < CIRCLE_CHECKPOINTS; k++)
				if (hare->circle_checkpoint_hit[k])
				{
					if (spCos(k*2*SP_PI/CIRCLE_CHECKPOINTS - hare->rotation - SP_PI/2) < SP_ONE/4)
					{
						hare->dx += spCos(k*2*SP_PI/CIRCLE_CHECKPOINTS + SP_PI) >> 8;
						hare->dy += spSin(k*2*SP_PI/CIRCLE_CHECKPOINTS + SP_PI) >> 8;
					}
					else
						hare->bums = 1;
				}
			}
			if (((hase_game->options.bytewise.ragnarok_border & 15) == 0) && (hare->x <  0 || hare->y < 0 || hare->x >= spIntToFixed(LEVEL_WIDTH) || hare->y >= spIntToFixed(LEVEL_HEIGHT)))
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
				if (alive_count < 2)
					return 1;
			}
			else
			{
				if (hase_game->options.bytewise.ragnarok_border & 15)
				{
					if (hare->x < 0)
						hare->x += spIntToFixed(LEVEL_WIDTH);
					if (hare->y < 0)
						hare->y += spIntToFixed(LEVEL_HEIGHT);
					if (hare->x >= spIntToFixed(LEVEL_WIDTH))
						hare->x -= spIntToFixed(LEVEL_WIDTH);
					if (hare->y >= spIntToFixed(LEVEL_HEIGHT))
						hare->y -= spIntToFixed(LEVEL_HEIGHT);
				}
				hare = hare->next;
			}
		}
		while (hare != player[j]->firstHare);
	}
	return updateBullets();
}
