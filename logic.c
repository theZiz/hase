int do_physics(int steps)
{
	int i,j;
	for (i = 0; i < steps; i++)
		for (j = 0; j < 2; j++)
		{
			player[j].dx -= gravitation_x(player[j].x >> SP_ACCURACY,player[j].y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			player[j].dy -= gravitation_y(player[j].x >> SP_ACCURACY,player[j].y >> SP_ACCURACY) >> PHYSIC_IMPACT;
			if (circle_is_empty(player[j].x+player[j].dx >> SP_ACCURACY,player[j].y+player[j].dy >> SP_ACCURACY,7))
			{
				player[j].x += player[j].dx;
				player[j].y += player[j].dy;
			}
			else
			{
				player[j].dx = 0;
				player[j].dy = 0;
				player[j].bums = 1;
			}
		}
	return updateBullets(steps);
}
