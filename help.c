#define HELP_DISTANCE font->maxheight/2

void draw_help()
{
	if (help == 0)
	{
		spFontDrawRight( screen->w-1, 0, 0, "[R]Help", font );
		return;
	}
	int h = 31*HELP_DISTANCE+4;
	int lw = spMax(spFontWidth("[4]+[<]/[^]/[>]/[v]: ",font),spFontWidth("[4]+[o]/[c]/[3]: ",font));
	int rw = spFontWidth("direction of the weapon",font);
	int w = lw + rw + 4;
	spSetBlending(SP_ONE/2);
	spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
	spSetBlending(SP_ONE);
	spSetPattern8(153,60,102,195,153,60,102,195);
	spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
	spDeactivatePattern();

	spFontDraw     ( 2+lw, screen->h-2-31*HELP_DISTANCE, 0, "Controls", font );

	spFontDrawRight( 2+lw, screen->h-2-29*HELP_DISTANCE, 0, "[<]/[>]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-29*HELP_DISTANCE, 0, "direction & low jumps", font );
	spFontDrawRight( 2+lw, screen->h-2-27*HELP_DISTANCE, 0, "[^]/[v]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-27*HELP_DISTANCE, 0, "direction of the weapon", font );
	spFontDrawRight( 2+lw, screen->h-2-25*HELP_DISTANCE, 0, "[3]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-25*HELP_DISTANCE, 0, "Choose weapon", font );
	spFontDrawRight( 2+lw, screen->h-2-23*HELP_DISTANCE, 0, "[c]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-23*HELP_DISTANCE, 0, "Shoot!", font );
	spFontDrawRight( 2+lw, screen->h-2-21*HELP_DISTANCE, 0, "[l]/[r]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-21*HELP_DISTANCE, 0, "Weapon strength!", font );
	spFontDrawRight( 2+lw, screen->h-2-19*HELP_DISTANCE, 0, "[o]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-19*HELP_DISTANCE, 0, "High jump", font );
	spFontDrawRight( 2+lw, screen->h-2-17*HELP_DISTANCE, 0, "[B]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-17*HELP_DISTANCE, 0, "Options and Exit", font );
	spFontDrawRight( 2+lw, screen->h-2-15*HELP_DISTANCE, 0, "[R]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-15*HELP_DISTANCE, 0, "Hide help", font );

	spFontDraw     ( 2+lw, screen->h-2-12*HELP_DISTANCE, 0, "Visual settings", font );

	spFontDrawRight( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "[4]+[l]/[r]: ", font );
	spFontDraw     ( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "Zoom in or out", font );
	spFontDrawRight( 2+lw, screen->h-2- 8*HELP_DISTANCE, 0, "[4]+[<]/[^]/[>]/[v]: ", font );
	spFontDraw     ( 2+lw, screen->h-2- 8*HELP_DISTANCE, 0, "Look around", font );
	spFontDrawRight( 2+lw, screen->h-2- 6*HELP_DISTANCE, 0, "[4]+[o]: ", font );
	spFontDraw     ( 2+lw, screen->h-2- 6*HELP_DISTANCE, 0, "Chat", font );
	spFontDrawRight( 2+lw, screen->h-2- 4*HELP_DISTANCE, 0, "[4]+[3]: ", font );
	spFontDraw     ( 2+lw, screen->h-2- 4*HELP_DISTANCE, 0, "Show/Hide player names", font );
	spFontDrawRight( 2+lw, screen->h-2- 2*HELP_DISTANCE, 0, "[4]+[c]: ", font );
	spFontDraw     ( 2+lw, screen->h-2- 2*HELP_DISTANCE, 0, "Show/Hide mini map", font );

}
