void draw_help()
{
	if (help == 0)
	{
		spFontDrawMiddle( screen->w/2, 1, 0, "Press [R] for help", font );
		return;
	}
	spSetAlphaPattern4x4(127,0);
	spRectangle( screen->w/2,screen->h/4,0,screen->w*2/3,screen->h/2,spGetRGB(127,127,127));
	spDeactivatePattern();
	spFontDrawMiddle( screen->w/2, 2+0*font->maxheight, 0, "[R]: Close help", font );
	spFontDrawMiddle( screen->w/2, 2+2*font->maxheight, 0, "[a]: Low jump", font );
	spFontDrawMiddle( screen->w/2, 2+3*font->maxheight, 0, "[d]: high jump", font );
	spFontDrawMiddle( screen->w/2, 2+4*font->maxheight, 0, "[s]: Change bunny direction", font );
	spFontDrawMiddle( screen->w/2, 2+5*font->maxheight, 0, SP_PAD_NAME" Left & Right: Power", font );
	spFontDrawMiddle( screen->w/2, 2+6*font->maxheight, 0, SP_PAD_NAME" Up & Down: Shoot Direction", font );
	spFontDrawMiddle( screen->w/2, 2+7*font->maxheight, 0, "[w]: Shoot!", font );
	spFontDrawMiddle( screen->w/2, 2+9*font->maxheight, 0, "[B] Exit", font );
}
