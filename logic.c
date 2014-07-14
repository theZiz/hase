int do_physics(int steps)
{
	int i,j;
	for (i = 0; i < steps; i++)
		for (j = 0; j < player_count; j++)
		{
			if (player[j]->health == 0)
				continue;
			player[j]->dx -= gravitation_x(player[j]->x >> SP_ACCURACY,player[j]->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			player[j]->dy -= gravitation_y(player[j]->x >> SP_ACCURACY,player[j]->y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			if (circle_is_empty(player[j]->x+player[j]->dx >> SP_ACCURACY,player[j]->y+player[j]->dy >> SP_ACCURACY,8,j))
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
			}
		}
	return updateBullets(steps);
}
