void do_physics(int steps)
{
	int i;
	for (i = 0; i < steps; i++)
	{
		player.dx -= gravitation_x(player.x >> SP_ACCURACY,player.y >> SP_ACCURACY)/8192;
		player.dy -= gravitation_y(player.x >> SP_ACCURACY,player.y >> SP_ACCURACY)/8192;
		if (circle_is_empty(player.x+player.dx >> SP_ACCURACY,player.y+player.dy >> SP_ACCURACY,15))
		{
			player.x += player.dx;
			player.y += player.dy;
		}
		else
		{
			player.dx = 0;
			player.dy = 0;
			player.bums = 1;
		}
	}
}
