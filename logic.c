int do_physics()
{
	int j;
	for (j = 0; j < player_count; j++)
	{
		if (player[j]->health == 0)
			continue;
		player[j]->dx -= gravitation_x(spFixedToInt(player[j]->x),spFixedToInt(player[j]->y)) >> PHYSIC_IMPACT;
		player[j]->dy -= gravitation_y(spFixedToInt(player[j]->x),spFixedToInt(player[j]->y)) >> PHYSIC_IMPACT;
		if (circle_is_empty(spFixedToInt(player[j]->x+player[j]->dx),spFixedToInt(player[j]->y+player[j]->dy),8,j))
		{
			player[j]->x += player[j]->dx;
			player[j]->y += player[j]->dy;
		}
		else
		{
			player[j]->dx = 0;
			player[j]->dy = 0;
			player[j]->bums = 1;
		}
		if (player[j]->x <  0           || player[j]->y < 0 ||
			player[j]->x >= spIntToFixed(LEVEL_WIDTH) || player[j]->y >= spIntToFixed(LEVEL_HEIGHT))
		{
			player[j]->health = 0;
			alive_count--;
			if (alive_count < 2)
				return 1;
			if (j == active_player)
				next_player();
		}
	}
	return updateBullets();
}
