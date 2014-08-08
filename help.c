void draw_help()
{
	if (help == 0)
	{
		spFontDrawMiddle( screen->w/2, screen->h-2-font->maxheight, 0, "Press [R] for help", font );
		return;
	}
	spSetAlphaPattern4x4(127,0);
	int h = 10*font->maxheight+4;
	spRectangle( screen->w/2,screen->h-h/2,0,screen->w*7/8,h,spGetRGB(127,127,127));
	spDeactivatePattern();
	spFontDrawMiddle( screen->w/2, screen->h-2-10*font->maxheight, 0, "[R]: Close help", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 8*font->maxheight, 0, SP_PAD_NAME" Left & Right: direction & low jumps", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 7*font->maxheight, 0, "[o]: high jump", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 6*font->maxheight, 0, "[B] Choose weapon", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 5*font->maxheight, 0, SP_PAD_NAME" Up & Down: direction of the weapon", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 4*font->maxheight, 0, "[3] & [4]: Weapon strength!", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 3*font->maxheight, 0, "[c]: Shoot!", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 2*font->maxheight, 0, "[l] & [r]: Zoom in & out", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 1*font->maxheight, 0, "[R][B]: Exit", font );
}
