int do_physics()
{
	int j;
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
			{
				hare->dx = 0;
				hare->dy = 0;
				hare->bums = 1;
			}
			if (hare->x <  0           || hare->y < 0 ||
				hare->x >= spIntToFixed(LEVEL_WIDTH) || hare->y >= spIntToFixed(LEVEL_HEIGHT))
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
				hare = hare->next;
		}
		while (hare != player[j]->firstHare);
	}
	return updateBullets();
}
