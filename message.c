#include "message.h"
#include "lobbyList.h"

int mg_button_count;
char mg_caption[256];
spFontPointer mg_font;
char* mg_change_char;
int mg_pos;
int mg_sel;
int *mg_players;
char *mg_name;
int *mg_seconds;
int *mg_online;


void message_draw(void)
{
	SDL_Surface* screen = spGetWindowSurface();
	spSetAlphaPattern4x4(196,0);
	spRectangle(screen->w/2,screen->h/2,0,screen->w,screen->h,LL_BG);
	spDeactivatePattern();
	char buffer[256];
	if (mg_button_count < 0)
	{
		spRectangle(screen->w/2,screen->h/2,0,screen->w/2,screen->h/2,LL_BG);
		int meow = spGetSizeFactor()*4 >> SP_ACCURACY;
		spRectangleBorder(screen->w/2,screen->h/2,0,screen->w/2+2*meow,screen->h/2+2*meow,meow,meow,LL_FG);
		spFontDrawMiddle( screen->w/2, 6*screen->h/23, 0, "Create game", mg_font );
		sprintf(buffer,"Name: %s",mg_name);
		spFontDrawMiddle( screen->w/2, 8*screen->h/23, 0, buffer, mg_font );
		sprintf(buffer,"Maximum players: %i",*mg_players);
		spFontDrawMiddle( screen->w/2,10*screen->h/23, 0, buffer, mg_font );
		sprintf(buffer,"Seconds per turn: %i",*mg_seconds);
		spFontDrawMiddle( screen->w/2,12*screen->h/23, 0, buffer, mg_font );
		switch (*mg_online)
		{
			case -1:
				sprintf(buffer,"Local game (no internet)");
				break;
			case 0:
				sprintf(buffer,"Local game");
				break;
			case 1:
				sprintf(buffer,"Online game");
				break;
		}
		spFontDrawMiddle( screen->w/2,14*screen->h/23, 0, buffer, mg_font );

		spFontDrawMiddle( screen->w/2, (8+mg_pos*2)*screen->h/23, 0, ">>                                           <<", mg_font );

		/*if (mg_change_char)
		{
			spFontDrawMiddle( screen->w/2, screen->h/2-mg_font->maxheight/2, 0, mg_change_char, mg_font );
			int x = (screen->w+spFontWidth(mg_change_char,mg_font))/2;
			if ((SDL_GetTicks() >> 9) & 1)
				spLine(x,screen->h/2-mg_font->maxheight/2, 0,x,screen->h/2+mg_font->maxheight/2, 0,65535);
		}*/
		if (mg_sel)
		{
			switch (mg_pos)
			{
				case 0:
					spFontDrawMiddle( screen->w/2,16*screen->h/23, 0, "Enter your name  [a]Back", mg_font );
					break;
				default:
					spFontDrawMiddle( screen->w/2,16*screen->h/23, 0, SP_PAD_NAME": Change  [a]Back", mg_font );
			}
		}
		else
			spFontDrawMiddle( screen->w/2,16*screen->h/23, 0, "[a]Select  [w]Create  [d]Cancel", mg_font );
		spFlip();
	}
	else
	{
		int width = screen->w/2;
		int meow = spGetSizeFactor()*4 >> SP_ACCURACY;
		if (mg_change_char)
			width = spMax(width,spFontWidth(mg_change_char, mg_font)+2*meow);
		spRectangle(screen->w/2,screen->h/2,0,width,screen->h/4,LL_BG);
		spRectangleBorder(screen->w/2,screen->h/2,0,width+2*meow,screen->h/4+2*meow,meow,meow,LL_FG);
		spFontDrawMiddle( screen->w/2, 3*screen->h/7, 0, mg_caption, mg_font );
		if (mg_change_char)
		{
			spFontDrawMiddle( screen->w/2, screen->h/2-mg_font->maxheight/2, 0, mg_change_char, mg_font );
			int x = (screen->w+spFontWidth(mg_change_char,mg_font))/2;
			if ((SDL_GetTicks() >> 9) & 1)
				spLine(x,screen->h/2-mg_font->maxheight/2, 0,x,screen->h/2+mg_font->maxheight/2, 0,65535);
		}
		switch (mg_button_count)
		{
			case 1:
				spFontDrawMiddle( screen->w/2, 4*screen->h/7-mg_font->maxheight, 0, "[a]Ok", mg_font );
				break;
			case 2:
				spFontDrawMiddle( screen->w/2, 4*screen->h/7-mg_font->maxheight, 0, "[a]Ok     [d]Cancel", mg_font );
				break;
		}
		if (mg_button_count > 0)
			spFlip();
	}
}

int message_calc(Uint32 steps)
{
	if (mg_button_count < 0)
	{
		if (spGetInput()->button[SP_BUTTON_RIGHT])
		{
			spGetInput()->button[SP_BUTTON_RIGHT] = 0;
			return 2;
		}
		if (spGetInput()->button[SP_BUTTON_UP])
		{
			spGetInput()->button[SP_BUTTON_UP] = 0;
			return 1;
		}
		if (spGetInput()->button[SP_BUTTON_LEFT])
		{
			spGetInput()->button[SP_BUTTON_LEFT] = 0;
			mg_sel = 1-mg_sel;
			if (mg_sel == 0 && mg_pos == 0)
				spStopKeyboardInput();
			if (mg_sel == 1 && mg_pos == 0)
				spPollKeyboardInput(mg_name,32,SP_BUTTON_DOWN_MASK);
		}
		if (mg_sel)
		{
			if (spGetInput()->axis[0] > 0 && mg_pos!=0)
			{
				spGetInput()->axis[0] = 0;
				switch (mg_pos)
				{
					case 1:
						(*mg_players)++;
						break;
					case 2:
						(*mg_seconds)+=5;
						break;
					case 3:
						if ((*mg_online) >= 0)
							(*mg_online) = 1-(*mg_online);
						break;
				}
			}
			if (spGetInput()->axis[0] < 0 && mg_pos!=0)
			{
				spGetInput()->axis[0] = 0;
				switch (mg_pos)
				{
					case 1:
						if ((*mg_players) > 2)
							(*mg_players)--;
						break;
					case 2:
						if ((*mg_seconds) > 5)
							(*mg_seconds)-=5;
						break;
					case 3:
						if ((*mg_online) >= 0)
							(*mg_online) = 1-(*mg_online);
						break;
				}
			}
		}
		else
		{
			if (spGetInput()->axis[1] > 0 && mg_pos < 3)
			{
				spGetInput()->axis[1] = 0;
				mg_pos++;
			}
			if (spGetInput()->axis[1] < 0 && mg_pos > 0)
			{
				spGetInput()->axis[1] = 0;
				mg_pos--;
			}
		}
	}
	if (mg_button_count > 0 && spGetInput()->button[SP_BUTTON_LEFT])
	{
		spGetInput()->button[SP_BUTTON_LEFT] = 0;
		return 1;
	}
	if (mg_button_count > 1 && spGetInput()->button[SP_BUTTON_RIGHT])
	{
		spGetInput()->button[SP_BUTTON_RIGHT] = 0;
		return 2;
	}

	return 0;
}

int message(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption,int button_count,char* change_char)
{
	mg_button_count = button_count;
	mg_font = font;
	sprintf(mg_caption,"%s",caption);
	mg_change_char = change_char;
	if (button_count > 0)
		spLoop(message_draw,message_calc,10,resize,NULL);
}

int message_cg(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),char* name,int* players,int* seconds,int* online)
{
	mg_pos = 0;
	mg_sel = 0;
	mg_button_count = -1;
	mg_font = font;
	mg_name = name;
	mg_players = players;
	mg_seconds = seconds;
	mg_online = online;
	
	spLoop(message_draw,message_calc,10,resize,NULL);
}
