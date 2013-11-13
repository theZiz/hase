spSpriteCollectionPointer hase;
struct
{
	int direction;
	Sint32 x,y;
} player;

void init_player()
{
	player.direction = 0;
	player.x = level->w << SP_ACCURACY -1;
	player.y = 0;
}
