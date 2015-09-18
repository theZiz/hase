#define HELP_DISTANCE font->maxheight/2

void draw_help()
{
	if (help == 0)
	{
		spFontDrawRight( screen->w-1, screen->h-font->maxheight*4-1, 0, "{view}+{jump}Help", font );
		return;
	}
	if (spGetSizeFactor() <= SP_ONE)
	{
		int h = 33*HELP_DISTANCE+4;
		int lw = spFontWidth("{view}+[<]/[^]/[>]/[v]: ",font);
		int rw = spFontWidth("direction of the weapon",font);
		int w = lw + rw + 4;
		spSetBlending(SP_ONE/2);
		spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
		spSetBlending(SP_ONE);
		spSetPattern8(153,60,102,195,153,60,102,195);
		spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
		spDeactivatePattern();
		spFontDraw ( 2+lw, screen->h-2-33*HELP_DISTANCE, 0, "Controls", font );
		spFontDrawRight( 2+lw, screen->h-2-31*HELP_DISTANCE, 0, "[<]/[>]: ", font );
		spFontDraw ( 2+lw, screen->h-2-31*HELP_DISTANCE, 0, "direction & low jumps", font );
		spFontDrawRight( 2+lw, screen->h-2-29*HELP_DISTANCE, 0, "[^]/[v]: ", font );
		spFontDraw ( 2+lw, screen->h-2-29*HELP_DISTANCE, 0, "direction of the weapon", font );
		spFontDrawRight( 2+lw, screen->h-2-27*HELP_DISTANCE, 0, "{weapon}: ", font );
		spFontDraw ( 2+lw, screen->h-2-27*HELP_DISTANCE, 0, "Choose weapon", font );
		spFontDrawRight( 2+lw, screen->h-2-25*HELP_DISTANCE, 0, "{shoot}: ", font );
		spFontDraw ( 2+lw, screen->h-2-25*HELP_DISTANCE, 0, "Shoot!", font );
		spFontDrawRight( 2+lw, screen->h-2-23*HELP_DISTANCE, 0, "{power_down}/{power_up}: ", font );
		spFontDraw ( 2+lw, screen->h-2-23*HELP_DISTANCE, 0, "Weapon strength!", font );
		spFontDrawRight( 2+lw, screen->h-2-21*HELP_DISTANCE, 0, "{jump}: ", font );
		spFontDraw ( 2+lw, screen->h-2-21*HELP_DISTANCE, 0, "jump", font );
		spFontDrawRight( 2+lw, screen->h-2-19*HELP_DISTANCE, 0, "{jump}x2: ", font );
		spFontDraw ( 2+lw, screen->h-2-19*HELP_DISTANCE, 0, "high jump", font );
		spFontDrawRight( 2+lw, screen->h-2-17*HELP_DISTANCE, 0, "{menu}: ", font );
		spFontDraw ( 2+lw, screen->h-2-17*HELP_DISTANCE, 0, "Options and Exit", font );
		spFontDrawRight( 2+lw, screen->h-2-15*HELP_DISTANCE, 0, "{chat}: ", font );
		spFontDraw ( 2+lw, screen->h-2-15*HELP_DISTANCE, 0, "Chat (only online)", font );
		spFontDraw ( 2+lw, screen->h-2-12*HELP_DISTANCE, 0, "Visual settings", font );
		spFontDrawRight( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "{view}+{power_down}/{power_up}: ", font );
		spFontDraw ( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "Zoom in or out", font );
		spFontDrawRight( 2+lw, screen->h-2- 8*HELP_DISTANCE, 0, "{view}+[<]/[^]/[>]/[v]: ", font );
		spFontDraw ( 2+lw, screen->h-2- 8*HELP_DISTANCE, 0, "Look around", font );
		spFontDrawRight( 2+lw, screen->h-2- 6*HELP_DISTANCE, 0, "{view}+{weapon}: ", font );
		spFontDraw ( 2+lw, screen->h-2- 6*HELP_DISTANCE, 0, "Faster game", font );
		spFontDrawRight( 2+lw, screen->h-2- 4*HELP_DISTANCE, 0, "{view}+{shoot}: ", font );
		spFontDraw ( 2+lw, screen->h-2- 4*HELP_DISTANCE, 0, "Slower game", font );
		spFontDrawRight( 2+lw, screen->h-2- 2*HELP_DISTANCE, 0, "{view}+{jump}: ", font );
		spFontDraw ( 2+lw, screen->h-2- 2*HELP_DISTANCE, 0, "Hide help", font );
	}
	else
	{		
		int h = 20*HELP_DISTANCE+4;
		int lw = spFontWidth("{view}+[<]/[^]/[>]/[v]: ",font);
		int rw = spFontWidth("direction of the weapon",font);
		int w = lw + rw + 4;
		spSetBlending(SP_ONE/2);
		spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
		spSetBlending(SP_ONE);
		spSetPattern8(153,60,102,195,153,60,102,195);
		spRectangle( w/2,screen->h-h/2,0,w,h,LL_BG);
		spDeactivatePattern();

		spFontDraw     ( 2+lw, screen->h-2-20*HELP_DISTANCE, 0, "Controls", font );

		spFontDrawRight( 2+lw, screen->h-2-18*HELP_DISTANCE, 0, "[<]/[>]: ", font );
		spFontDraw     ( 2+lw, screen->h-2-18*HELP_DISTANCE, 0, "direction & low jumps", font );
		spFontDrawRight( 2+lw, screen->h-2-16*HELP_DISTANCE, 0, "[^]/[v]: ", font );
		spFontDraw     ( 2+lw, screen->h-2-16*HELP_DISTANCE, 0, "direction of the weapon", font );
		spFontDrawRight( 2+lw, screen->h-2-14*HELP_DISTANCE, 0, "{weapon}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-14*HELP_DISTANCE, 0, "Choose weapon", font );
		spFontDrawRight( 2+lw, screen->h-2-12*HELP_DISTANCE, 0, "{shoot}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-12*HELP_DISTANCE, 0, "Shoot!", font );
		spFontDrawRight( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "{power_down}/{power_up}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-10*HELP_DISTANCE, 0, "Weapon strength!", font );
		spFontDrawRight( 2+lw, screen->h-2-8*HELP_DISTANCE, 0, "{jump}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-8*HELP_DISTANCE, 0, "jump", font );
		spFontDrawRight( 2+lw, screen->h-2-6*HELP_DISTANCE, 0, "{jump}x2: ", font );
		spFontDraw     ( 2+lw, screen->h-2-6*HELP_DISTANCE, 0, "high jump", font );
		spFontDrawRight( 2+lw, screen->h-2-4*HELP_DISTANCE, 0, "{menu}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-4*HELP_DISTANCE, 0, "Options and Exit", font );
		spFontDrawRight( 2+lw, screen->h-2-2*HELP_DISTANCE, 0, "{chat}: ", font );
		spFontDraw     ( 2+lw, screen->h-2-2*HELP_DISTANCE, 0, "Chat (only online)", font );


		h = 12*HELP_DISTANCE+4;
		lw = spFontWidth("{view}+[<]/[^]/[>]/[v]: ",font);
		rw = spFontWidth("Visual settings",font);
		w = lw + rw + 4;
		spSetBlending(SP_ONE/2);
		spRectangle( screen->w-w/2,screen->h-h/2-3*font->maxheight,0,w,h,LL_BG);
		spSetBlending(SP_ONE);
		spSetPattern8(153,60,102,195,153,60,102,195);
		spRectangle( screen->w-w/2,screen->h-h/2-3*font->maxheight,0,w,h,LL_BG);
		spDeactivatePattern();
		
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight-12*HELP_DISTANCE, 0, "Visual settings", font );

		spFontDrawRight( screen->w-2-rw, screen->h-2-3*font->maxheight-10*HELP_DISTANCE, 0, "{view}+{power_down}/{power_up}: ", font );
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight-10*HELP_DISTANCE, 0, "Zoom in or out", font );
		spFontDrawRight( screen->w-2-rw, screen->h-2-3*font->maxheight- 8*HELP_DISTANCE, 0, "{view}+[<]/[^]/[>]/[v]: ", font );
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight- 8*HELP_DISTANCE, 0, "Look around", font );
		spFontDrawRight( screen->w-2-rw, screen->h-2-3*font->maxheight- 6*HELP_DISTANCE, 0, "{view}+{weapon}: ", font );
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight- 6*HELP_DISTANCE, 0, "Faster game", font );
		spFontDrawRight( screen->w-2-rw, screen->h-2-3*font->maxheight- 4*HELP_DISTANCE, 0, "{view}+{shoot}: ", font );
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight- 4*HELP_DISTANCE, 0, "Slower game", font );
		spFontDrawRight( screen->w-2-rw, screen->h-2-3*font->maxheight- 2*HELP_DISTANCE, 0, "{view}+{jump}: ", font );
		spFontDraw     ( screen->w-2-rw, screen->h-2-3*font->maxheight- 2*HELP_DISTANCE, 0, "Hide help", font );
	}
}
