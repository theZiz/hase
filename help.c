void draw_help()
{
	if (help == 0)
	{
		spFontDraw( 2, screen->h-font->maxheight-2, 0, "[R]Help", font );
		return;
	}
	spSetAlphaPattern4x4(127,0);
	int h = 10*font->maxheight+4;
	int w = spFontWidth(SP_PAD_NAME" Left & Right: direction & low jumps",font)+4;
	spRectangle( w/2,screen->h-h/2,0,w,h,spGetRGB(127,127,127));
	spDeactivatePattern();
	spFontDraw( 2, screen->h-2-10*font->maxheight, 0, SP_PAD_NAME" Left & Right: direction & low jumps", font );
	spFontDraw( 2, screen->h-2- 9*font->maxheight, 0, SP_PAD_NAME" Up & Down: direction of the weapon", font );
	spFontDraw( 2, screen->h-2- 8*font->maxheight, 0, "[3]Choose weapon", font );
	spFontDraw( 2, screen->h-2- 7*font->maxheight, 0, "[c]Shoot!", font );
	spFontDraw( 2, screen->h-2- 6*font->maxheight, 0, "[l]/[r]Weapon strength!", font );
	spFontDraw( 2, screen->h-2- 5*font->maxheight, 0, "[o]high jump", font );
	spFontDraw( 2, screen->h-2- 4*font->maxheight, 0, "[4]+[l]/[r]Zoom in or out", font );
	spFontDraw( 2, screen->h-2- 3*font->maxheight, 0, "[4]+"SP_PAD_NAME": Look around", font );
	spFontDraw( 2, screen->h-2- 2*font->maxheight, 0, "[B]Pause & Menu", font );
	spFontDraw( 2, screen->h-2- 1*font->maxheight, 0, "[R]Hide help", font );
}
