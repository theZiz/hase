void draw_help()
{
	if (help == 0)
	{
		spFontDrawMiddle( screen->w/2, screen->h-2-font->maxheight, 0, "Press [R] for help", font );
		return;
	}
	spSetAlphaPattern4x4(127,0);
	spRectangle( screen->w/2,screen->h*3/4,0,screen->w*7/8,screen->h/2,spGetRGB(127,127,127));
	spDeactivatePattern();
	spFontDrawMiddle( screen->w/2, screen->h-2- 1*font->maxheight, 0, "[R]: Close help", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 8*font->maxheight, 0, "[a]: Low jump", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 7*font->maxheight, 0, "[d]: high jump", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 6*font->maxheight, 0, SP_PAD_NAME" Left & Right: Power", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 5*font->maxheight, 0, SP_PAD_NAME" Up & Down: direction; hold: low jump", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 4*font->maxheight, 0, "[w]: hold: Load energy; released: Shoot!", font );
	spFontDrawMiddle( screen->w/2, screen->h-2- 3*font->maxheight, 0, "[q]&[e]: Zoom", font );
	spFontDrawMiddle( screen->w/2, screen->h-2-10*font->maxheight, 0, "[B] Exit", font );
}
