#include "window.h"
#include "lobbyList.h"
#include "options.h"
#include <stdlib.h>

pWindow mg_window = NULL;
spSpriteCollectionPointer window_sprite[SPRITE_COUNT];
SDL_Surface* logo;

void init_window_sprites()
{
	int i;
	for (i = 0; i < SPRITE_COUNT; i++)
	{
		char buffer[256];
		sprintf(buffer,"./sprites/hase%i.ssc",i+1);
		window_sprite[i] = spLoadSpriteCollection(buffer,NULL);
		spSelectSprite(window_sprite[i],"high jump right");
		spSetSpriteZoom(spActiveSprite(window_sprite[i]),SP_ONE/2+spGetSizeFactor()/4,SP_ONE/2+spGetSizeFactor()/4);
	}
}

int get_last_sprite()
{
	return gop_sprite()+1;
}

void quit_window_sprites()
{
	int i;
	for (i = 0; i < SPRITE_COUNT; i++)
		spDeleteSpriteCollection(window_sprite[i],0);
}	

void update_window_width(pWindow window)
{
	pWindowElement elem = window->firstElement;
	window->width = spMax(spGetWindowSurface()->h,spFontWidth(window->title,window->font)+(spGetSizeFactor()*4 >> SP_ACCURACY)*2);
	while (elem)
	{
		window->width = spMax(window->width,elem->width+(spGetSizeFactor()*4 >> SP_ACCURACY)*2);
		elem = elem->next;
	}
	window->width = spMax(spGetWindowSurface()->w*3/4,spMin(window->width,spGetWindowSurface()->w-(spGetSizeFactor()*4 >> SP_ACCURACY)));
}

#define SMALL_HACK ((spGetSizeFactor() <= SP_ONE)?4:0)

pWindow create_window(int ( *feedback )( pWindow window, pWindowElement elem, int action ),spFontPointer font,char* title)
{
	pWindow window = (pWindow)malloc(sizeof(tWindow));
	window->height = font->maxheight*4+(spGetSizeFactor()*4 >> SP_ACCURACY)*2-2*SMALL_HACK;
	window->font = font;
	window->feedback = feedback;
	window->selection = 0;
	window->firstElement = NULL;
	sprintf(window->title,"%s",title);
	update_window_width(window);
	window->do_flip = 1;
	window->main_menu = 0;
	window->oldScreen = NULL;
	window->only_ok = 0;
	window->count = 0;
	window->show_selection = 0;
	window->sprite_count = NULL;
	window->insult_button = 0;
	window->text_box_char = NULL;
	window->text_box_len = 0;
	window->cancel_to_no = 0;
	window->zig_zag = 1;
	window->sizeFactor = spGetSizeFactor();
	int i = SP_MAPPING_MAX+4;
	while (i --> 0 )
	{
		window->button[i].x = -1;
		window->button[i].y = -1;
		window->button[i].w = -1;
		window->button[i].h = -1;
	}
	window->was_pressed = 0;
	return window;
}

void update_elem_width(pWindowElement elem,pWindow window)
{
	elem->width = spFontWidth(elem->text,window->font);
}

pWindowElement add_window_element(pWindow window,int type,int reference)
{
	pWindowElement elem = (pWindowElement)malloc(sizeof(tWindowElement));
	pWindowElement mom = window->firstElement;
	pWindowElement last = NULL;
	while (mom)
	{
		last = mom;
		mom = mom->next;
	}
	if (last)
		last->next = elem;
	else
		window->firstElement = elem;
	elem->next = NULL;
	elem->type = type;
	elem->reference = reference;
	elem->text[0] = 0;
	elem->button.x = -1;
	elem->button.y = -1;
	elem->button.w = -1;
	elem->button.h = -1;
	elem->button.type = -1;
	if (window->feedback)
		window->feedback(window,elem,WN_ACT_UPDATE);
	update_elem_width(elem,window);
	update_window_width(window);
	window->count++;
	if (window->count < 14)
		window->height+=3*window->font->maxheight/2-SMALL_HACK;
	return elem;
}

pWindow recent_window = NULL;

void draw_edgy_rectangle(int x, int y, int * width, int * height, int B0, int B1, int B2,int B4)
{
	*width += 2*B1;
	int Y;
	for (Y = y; Y <= y + *height; ++Y)
	{
		int add = 0;
		if (Y < y + B1 || Y > y + *height - B1)
			add = B2;
		spLine( x - B1 + add, Y, 0, x - B1 + *width - add, Y, 0, LL_FG );
	}
}

static void window_draw_buttons(window_text_positon position, int x, int y, char const * const text__)
{
	int l = strlen(text__);
	char text_buffer[l+1];
	memcpy(text_buffer,text__,(l+1)*sizeof(char));
	char* text = text_buffer;
	int width = spFontWidth( text, recent_window->font );
	switch (position)
	{
		case LEFT:
			break;
		case MIDDLE:
			x -= width/2;
			break;
		case RIGHT:
			x -= width;
			break;
	}
	int i = -1;
	int in_button = 0;
	do
	{
		++i;
		int draw_text = 0;
		if (!in_button && (text[i] == '{' || text[i] == '['))
		{
			in_button = 1;
			draw_text = i > 0;
		}
		if (in_button && (text[i] == ' ' || text[i] == 0))
		{
			in_button = 0;
			draw_text = i > 0;
		}
		else
		if (text[i] == 0)
		{
			in_button = 1;
			draw_text = i > 0;
		}
		if (draw_text)
		{
			char temp = text[i];
			text[i] = 0;
			if (!in_button)
			{
				int B0 = spMax(spGetSizeFactor()>>17,1);
				int B1 = spMax(spGetSizeFactor()>>16,1);
				int B2 = spMax(spGetSizeFactor()>>15,1);
				int B4 = spMax(spGetSizeFactor()>>14,1);
				width = spFontWidth( text, recent_window->font );
				int height = recent_window->font->maxheight;
				draw_edgy_rectangle(x,y,&width,&height,B0,B1,B2,B4);
				int j = 0;
				for (; text[j] && text[j] != '}' && text[j] != ']';j++);
				if (text[j])
				{
					char temp2 = text[j];
					text[j] = 0;
					int id	= -1;
					if (temp2 == '}')
						id = spMapIDByName( &(text[1]) );
					else // ]
					{
						switch (text[j-1])
						{
							case '<': id = SP_MAPPING_MAX+0; break;
							case '^': id = SP_MAPPING_MAX+1; break;
							case '>': id = SP_MAPPING_MAX+2; break;
							case 'v': id = SP_MAPPING_MAX+3; break;
						}
					}
					if ( id >= 0 )
					{
						recent_window->button[id].x = x;
						recent_window->button[id].y = y;
						recent_window->button[id].w = width;
						recent_window->button[id].h = height;
					}
					text[j] = temp2;
				}
			}
			spFontDraw( x, y, 0, text, recent_window->font );
			x += spFontWidth( text, recent_window->font );
			text[i] = temp;
			text = &(text[i]);
			i = -1;
		}
	}
	while (text[i]);
}

void window_draw(void)
{
	Sint32 sizeFactor = spGetSizeFactor();
	if (recent_window->sizeFactor != sizeFactor)
	{
		pWindowElement elem = recent_window->firstElement;
		while (elem)
		{
			elem->width = elem->width * sizeFactor / recent_window->sizeFactor;
			elem = elem->next;
		}
		recent_window->width = recent_window->width * sizeFactor / recent_window->sizeFactor;
		recent_window->height = recent_window->height * sizeFactor / recent_window->sizeFactor;
		recent_window->sizeFactor = sizeFactor;
	}
	SDL_Surface* screen = spGetWindowSurface();
	if (recent_window->oldScreen)
		spBlitSurface(screen->w/2,screen->h/2,0,recent_window->oldScreen);
	if (recent_window->zig_zag)
	{
		spSetPattern8(153,60,102,195,153,60,102,195);
		spRectangle(screen->w/2,screen->h/2,0,screen->w,screen->h,LL_BG);
		spDeactivatePattern();
	}

	if (spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
		spBlitSurface(screen->w/2,screen->h-spGetVirtualKeyboard()->h/2,0,spGetVirtualKeyboard());

	char buffer[256];
	pWindow window = recent_window;
	int meow = sizeFactor*4 >> SP_ACCURACY;
	
	int extra_y = 0;
	if (window->main_menu)
	{
		extra_y += logo->h/2;
		spBlitSurface(screen->w/2,screen->h/2 - extra_y * 2,0,logo);
		Sint32 zoom = spActiveSprite(window_sprite[gop_sprite()])->zoomX;
		spSetSpriteZoom(spActiveSprite(window_sprite[gop_sprite()]),spGetSizeFactor(),spGetSizeFactor());
		spDrawSprite(screen->w/2 - logo->h/4,screen->h/2 - extra_y * 2  - logo->h/4,0, spActiveSprite(window_sprite[gop_sprite()]));
		spSetSpriteZoom(spActiveSprite(window_sprite[gop_sprite()]),zoom,zoom);
	}
	if (recent_window->zig_zag == 0)
		extra_y -= spGetWindowSurface()->h/4;
	
	spRectangle(screen->w/2,screen->h/2 + extra_y,0,window->width-2*meow,window->height-2*meow,LL_BG);
	spRectangleBorder(screen->w/2,screen->h/2 + extra_y,0,window->width,window->height,meow,meow,LL_FG);
	int y = (screen->h - window->height) / 2 + meow + window->font->maxheight/2 + extra_y;
	spFontDrawMiddle( screen->w/2, y , 0, window->title, window->font );
	pWindowElement elem = window->firstElement;
	pWindowElement selElem = NULL;
	int nr = 0;
	int start_nr = 0;
	if (window->count > 13)
		start_nr = window->selection*(window->count-11)/window->count;
	int in_nr = 0;
	while (elem)
	{
		if (window->count < 14 || (nr >= start_nr && nr < start_nr + 13))
		{
			y+=window->font->maxheight*3/2-SMALL_HACK;
			int t_w = spFontWidth(elem->text,window->font);
			int t_r = (screen->w+t_w)/2;
			if (t_r > screen->w - meow)
				t_r = screen->w - meow;
			if (nr == window->selection)
			{
				if (window->firstElement->next) //more than one element
					spRectangle( screen->w/2, y+window->font->maxheight/2+SMALL_HACK/2, 0, window->width-2*meow,window->font->maxheight,LL_FG);
				selElem = elem;
				if (elem->type == 1 && spIsKeyboardPolled())
				{
					if ((SDL_GetTicks() >> 9) & 1)
						spLine(t_r, y, 0, t_r, y+window->font->maxheight, 0,65535);
				}
			}
			elem->button.w = window->width-2*meow;
			elem->button.h = window->font->maxheight;
			elem->button.x = (screen->w - elem->button.w)/2;
			elem->button.y = y+SMALL_HACK/2;
			if (start_nr > 0 && in_nr == 0)
			{
				t_w = spFontWidth("...",window->font);
				t_r = (screen->w+t_w)/2;
				spFontDrawRight( t_r, y, 0, "...", window->font );
				elem->button.type = 1;
			}
			else
			if (start_nr+13 < window->count && in_nr == 12)
			{
				t_w = spFontWidth("...",window->font);
				t_r = (screen->w+t_w)/2;
				spFontDrawRight( t_r, y, 0, "...", window->font );
				elem->button.type = 2;
			}
			else
			{
				spFontDrawRight( t_r, y, 0, elem->text, window->font );
				if (nr == window->selection && (elem->type == 0 || elem->type == 2))
				{
					window_draw_buttons( LEFT, t_r, y+SMALL_HACK/2, "  [>]" );
					window_draw_buttons( RIGHT, t_r - t_w, y+SMALL_HACK/2, "[<]  " );
				}
				elem->button.type = 0;
			}
			in_nr++;
		}
		else
		{
			elem->button.x = -1;
			elem->button.y = -1;
			elem->button.w = -1;
			elem->button.h = -1;
			elem->button.type = -1;
		}
		nr++;
		elem = elem->next;
	}
	
	if (window->show_selection)
	{
		y+=(sizeFactor*8 >> SP_ACCURACY)+window->font->maxheight*3/2-SMALL_HACK;
		int to_left = -spFontWidth("{power_up}",window->font);
		if (window->sprite_count && window->sprite_count[gop_sprite()])
		{
			if (window->sprite_count[gop_sprite()] == 1)
				sprintf(buffer,"{power_up} (already used one time)");
			else
				sprintf(buffer,"{power_up} (already used %i times)",window->sprite_count[gop_sprite()]);
		}
		else
			sprintf(buffer,"{power_up}");
		to_left += spFontWidth(buffer,window->font);
		to_left /= 2;
		spDrawSprite(screen->w/2-to_left, y, 0, spActiveSprite(window_sprite[gop_sprite()]));
		window_draw_buttons( RIGHT, screen->w/2-to_left-(sizeFactor*12 >> SP_ACCURACY), y-window->font->maxheight/2, "{power_down}" );
		window_draw_buttons(  LEFT, screen->w/2-to_left+(sizeFactor*12 >> SP_ACCURACY), y-window->font->maxheight/2, buffer );
		//spFontDrawRight( screen->w/2-to_left-(sizeFactor*12 >> SP_ACCURACY), y-window->font->maxheight/2, 0, "{power_down}", window->font );
		//spFontDraw     ( screen->w/2-to_left+(sizeFactor*12 >> SP_ACCURACY), y-window->font->maxheight/2, 0, buffer, window->font );
		y+=(sizeFactor*8 >> SP_ACCURACY)-SMALL_HACK;
		if (spGetSizeFactor() <= SP_ONE)
			y+=(sizeFactor*6 >> SP_ACCURACY);
		sprintf(buffer,"\"%s\"",window_sprite[gop_sprite()]->comment);
		spFontDrawMiddle( screen->w/2, y, 0, buffer, window->font);
		sprintf(buffer,"%s (%s)",window_sprite[gop_sprite()]->author,window_sprite[gop_sprite()]->license);
		y += window->font->maxheight-SMALL_HACK;
		spFontDrawMiddle( screen->w/2, y, 0, buffer, window->font);
	}
	
	y = (screen->h + window->height) / 2 - meow - 3*window->font->maxheight/2-SMALL_HACK/2  + extra_y + 1;
	if (selElem)
	{
		switch (selElem->type)
		{
			case 0: case 2:
				if (window->only_ok)
					window_draw_buttons( MIDDLE, screen->w/2,y, SP_PAD_NAME": Change  {jump}Okay" );
				else
					window_draw_buttons( MIDDLE, screen->w/2,y, SP_PAD_NAME": Change  {jump}Okay  {shoot}Cancel" );
				break;
			case 1:
				if (spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
				{
					if (spIsKeyboardPolled())
					{
						if (window->only_ok)
						{
							if (window->firstElement->next)
								window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Enter letter {view}<- {chat}Back" );
							else
								window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Enter letter {view}<- {chat}Okay" );
						}
						else
						{
							if (window->firstElement->next)
								window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Enter letter {view}<- {chat}/{shoot}Back" );
							else
							if (window->insult_button)
								window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Letter {view}<- {chat}Ok {shoot}Cancel {weapon}Insult" );
							else
								window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Enter letter {view}<- {chat}Okay {shoot}Cancel" );
						}
					}
					else
					if (window->only_ok)
						window_draw_buttons( MIDDLE, screen->w/2,y, "{chat}Change  {jump}Okay" );
					else
						window_draw_buttons( MIDDLE, screen->w/2,y, "{chat}Change  {jump}Okay  {shoot}Cancel" );
				}
				else
				if (window->only_ok)
					window_draw_buttons( MIDDLE, screen->w/2,y, "Keyboard: Change  {jump}Okay" );
				else
				if (window->insult_button)
					window_draw_buttons( MIDDLE, screen->w/2,y, "Keyboard: Change  {jump}Okay  {shoot}Cancel  {weapon}Insult" );
				else
					window_draw_buttons( MIDDLE, screen->w/2,y, "Keyboard: Change  {jump}Okay  {shoot}Cancel" );
				break;
			case -1:
				if (window->main_menu)
					window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Select  {shoot}Exit" );
				else
				if (window->only_ok)
					window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Select  {shoot}Back" );
				else
				if (window->cancel_to_no)
					window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Yes  {shoot}No" );
				else
					window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Okay  {shoot}Cancel" );
				break;
			case -2:
				window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Okay" );
				break;
		}
	}
	else
	if (window->do_flip)
	{
		if (window->only_ok)
			window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Okay" );
		else
			window_draw_buttons( MIDDLE, screen->w/2,y, "{jump}Okay  {shoot}Cancel" );
	}
	if (window->do_flip)
		spFlip();
}

void fill_with_insult(char* buffer)
{
	switch (rand()%60)
	{
		case  0: sprintf(buffer,"Son of a witch!"); break;
		case  1: sprintf(buffer,"Your mama's gravity made me miss!"); break;
		case  2: sprintf(buffer,"\"Worm with a hat\"-head!"); break;
		case  3: sprintf(buffer,"No you're a Craigix!"); break;
		case  4: sprintf(buffer,"Mostly harmless"); break;
		case  5: sprintf(buffer,"You and who's army?"); break;
		case  6: sprintf(buffer,"I've seen bigger"); break;
		case  7: sprintf(buffer,"Ha Ha!"); break;
		case  8: sprintf(buffer,"You're history"); break;
		case  9: sprintf(buffer,"Tree Hugger"); break;
		case 10: sprintf(buffer,"Talk to the hand"); break;
		case 11: sprintf(buffer,"I'm laughing on the inside"); break;
		case 12: sprintf(buffer,"You'z dead meat!"); break;
		case 13: sprintf(buffer,"Over here"); break;
		case 14: sprintf(buffer,"Want some candy?"); break;
		case 15: sprintf(buffer,"One dead dude coming up"); break;
		case 16: sprintf(buffer,"Is that the way to Amarillo?"); break;
		case 17: sprintf(buffer,"Back off cupcake"); break;
		case 18: sprintf(buffer,"Is that all you've got?"); break;
		case 19: sprintf(buffer,"You fight like a cow!"); break;
		case 20: sprintf(buffer,"I'm out of touch, you're out of time!"); break;
		case 21: sprintf(buffer,"It really is time for you to go to the occulist."); break;
		case 22: sprintf(buffer,"You call that shooting? Watch how it's done."); break;
		case 23: sprintf(buffer,"Mwahahaha... *ahem*."); break;
		case 24: sprintf(buffer,"parabolic math too hard for ya?"); break;
		case 25: sprintf(buffer,"Revenge is a dish best served... right NOW!"); break;
		case 26: sprintf(buffer,"You shoot like a jellyfish!"); break;
		case 27: sprintf(buffer,"y00 sux0r n00b!"); break;
		case 28: sprintf(buffer,"It seems you do not understand gravity."); break;
		case 29: sprintf(buffer,"I got 99 problems, but you ain’t one."); break;
		case 30: sprintf(buffer,"Dognabbit!"); break;
		case 31: sprintf(buffer,"Son of a beach!"); break;
		case 32: sprintf(buffer,"I'll get you next time!"); break;
		case 33: sprintf(buffer,"nya nya, didn't hurt at all!"); break;
		case 34: sprintf(buffer,"My granny can shoot better than that"); break;
		case 35: sprintf(buffer,"Don't point that thing at me"); break;
		case 36: sprintf(buffer,"Sure, blame it on your ISP"); break;
		case 37: sprintf(buffer,"I will see you in two months!"); break;
		case 38: sprintf(buffer,"You suck as much as the Pandora's Wifi chip!"); break;
		case 39: sprintf(buffer,"Cheater!"); break;
		case 40: sprintf(buffer,"What's your order number?"); break;
		case 41: sprintf(buffer,"You're weaker than a Pandora case!"); break;
		case 42: sprintf(buffer,"Go get yourself an Alu case?"); break;
		case 43: sprintf(buffer,"But then 2 monthsTM you have to wait!"); break;
		case 44: sprintf(buffer,"Hase hole."); break;
		case 45: sprintf(buffer,"I'm all outa gum..."); break;
		case 46: sprintf(buffer,"Your face your ass what's the difference?"); break;
		case 47: sprintf(buffer,"Come get some!"); break;
		case 48: sprintf(buffer,"Do you want to wash Wang or do you want to see Wang wash wang?"); break;
		case 49: sprintf(buffer,"Hail to the king Baby."); break;
		case 50: sprintf(buffer,"You wanna dance?"); break;
		case 51: sprintf(buffer,"You couldn't hit a planet if you were standing on it."); break;
		case 52: sprintf(buffer,"systemd lover!"); break;
		case 53: sprintf(buffer,"Gnome lover!"); break;
		case 54: sprintf(buffer,"KDE lover!"); break;
		case 55: sprintf(buffer,"Unity lover!"); break;
		case 56: sprintf(buffer,"A chicken could hit me, if it would press {shoot} enough!"); break;
		case 57: sprintf(buffer,"Where is your god now?"); break;
		case 58: sprintf(buffer,"No, go away!"); break;
		case 59: sprintf(buffer,"Jet fuel doesn't melt steel beams!"); break;
	}
}

int window_pause;
int last_window_pause;

int window_calc(Uint32 steps)
{
	pWindow window = recent_window;
	if ( spGetInput()->touchscreen.pressed )
	{
		window->was_pressed = 1;
		int mx = spGetInput()->touchscreen.x;
		int my = spGetInput()->touchscreen.y;
		pWindowElement elem = window->firstElement;
		int nr = 0;
		while (elem)
		{
			if ( elem->button.x + elem->button.w >= mx &&
				elem->button.x <= mx &&
				elem->button.y + elem->button.h >= my &&
				elem->button.y <= my )
			{
				switch (elem->button.type)
				{
					case 0:
						window->selection = nr;
						if (elem->type == -1)
							spMapSetByID( MAP_JUMP, 1 );
						break;
					case 1:
						spGetInput()->axis[1] = -1;
						break;
					case 2:
						spGetInput()->axis[1] = +1;
						break;
				}
				spGetInput()->touchscreen.pressed = 0;
				break;
			}
			nr++;
			elem = elem->next;
		}
		int i = SP_MAPPING_MAX+4;
		while (i --> 0 )
		{
			if ( window->button[i].x + window->button[i].w >= mx &&
				window->button[i].x <= mx &&
				window->button[i].y + window->button[i].h >= my &&
				window->button[i].y <= my )
			{
				if ( i < SP_MAPPING_MAX )
				{
					spMapSetByID( i, 1 );
					spGetInput()->touchscreen.pressed = 0;
				}
				else
					switch ( i )
					{
						case SP_MAPPING_MAX + 0:
							spGetInput()->axis[0] = -1;
							break;
						case SP_MAPPING_MAX + 1:
							spGetInput()->axis[1] = -1;
							break;
						case SP_MAPPING_MAX + 2:
							spGetInput()->axis[0] = 1;
							break;
						case SP_MAPPING_MAX + 3:
							spGetInput()->axis[1] = 1;
							break;
					}
				break;
			}
		}
	}
	else
	if (window->was_pressed)
	{
		window->was_pressed = 0;
		spGetInput()->axis[0] = 0;
		spGetInput()->axis[1] = 0;
	}
	if (window->insult_button && spMapGetByID(MAP_WEAPON) && spIsKeyboardPolled())
	{
		spMapSetByID(MAP_WEAPON,0);
		char buffer[128];
		fill_with_insult(buffer);
		snprintf(&(spGetInput()->keyboard.buffer[spGetInput()->keyboard.pos]),spGetInput()->keyboard.len-spGetInput()->keyboard.pos,"%s",buffer);
		spGetInput()->keyboard.pos += strlen(buffer);
		if (spGetInput()->keyboard.pos > spGetInput()->keyboard.len)
			spGetInput()->keyboard.pos = spGetInput()->keyboard.len;
		spGetInput()->keyboard.lastSize = 1;
	}
	
	if (window->show_selection || window->main_menu)
		spUpdateSprite(spActiveSprite(window_sprite[gop_sprite()]),steps);	
	if (window->show_selection)
	{
		if (spMapGetByID(MAP_POWER_DN))
		{
			spMapSetByID(MAP_POWER_DN,0);
			sop_sprite((gop_sprite() + SPRITE_COUNT - 1) % SPRITE_COUNT);
		}
		if (spMapGetByID(MAP_POWER_UP))
		{
			spMapSetByID(MAP_POWER_UP,0);
			sop_sprite((gop_sprite() + 1) % SPRITE_COUNT);
		}
	}
	pWindowElement selElem = window->firstElement;
	pWindowElement befElem = NULL;
	int nr = 0;
	while (selElem)
	{
		if (nr == window->selection)
			break;
		nr++;
		befElem = selElem;
		selElem = selElem->next;
	}
	if (selElem	== NULL)
	{
		if (spMapGetByID(MAP_JUMP) || spMapGetByID(MAP_CHAT) || spMapGetByID(MAP_MENU))
		{
			spMapSetByID(MAP_JUMP,0);
			spMapSetByID(MAP_CHAT,0);
			spMapSetByID(MAP_MENU,0);
			return 1;
		}
		if (window->only_ok == 0 && spMapGetByID(MAP_SHOOT))
		{
			spMapSetByID(MAP_SHOOT,0);
			return 2;
		}
		return 0;
	}
	if (befElem == NULL)
	{
		befElem = window->firstElement;
		while (befElem->next)
			befElem = befElem->next;
	}
	if (selElem->type != 1 ||
		!spIsKeyboardPolled() ||
		spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
	{
		int i;
		for (i = 0; i < steps; i++)
		{
			if (spGetInput()->axis[1] < 0)
			{
				if (window_pause == 0)
				{
					if (selElem->type == 1 &&
						spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER &&
						window->feedback)
						window->feedback(window,selElem,WN_ACT_END_POLL);
					window->selection = (window->selection + window->count - 1) % window->count;
					selElem = befElem;
					last_window_pause -= 25;
					if (last_window_pause < 25)
						last_window_pause = 25;
					window_pause = last_window_pause;
				}
				else
					window_pause--;
			}
			else
			if (spGetInput()->axis[1] > 0)
			{
				if (window_pause == 0)
				{
					if (selElem->type == 1 &&
						spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER &&
						window->feedback)
						window->feedback(window,selElem,WN_ACT_END_POLL);
					window->selection = (window->selection + 1) % window->count;
					if (selElem->next)
						selElem = selElem->next;
					else
						selElem = window->firstElement;
					last_window_pause -= 25;
					if (last_window_pause < 25)
						last_window_pause = 25;
					window_pause = last_window_pause;
				}
				else
					window_pause--;
			}
			else
			{
				last_window_pause = 300;
				window_pause = 0;
			}
		}
	}
	if ((spMapGetByID(MAP_JUMP) && (spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER || spIsKeyboardPolled() == 0 ))||
		(spMapGetByID(MAP_MENU) && window->only_ok && selElem->type != -1))
	{
		spMapSetByID(MAP_JUMP,0);
		if (window->only_ok && selElem->type != -1)
			spMapSetByID(MAP_MENU,0);
		if (selElem->type == 1)
		{
			if (spIsKeyboardPolled())
			{
				if (window->feedback)
					window->feedback(window,selElem,WN_ACT_END_POLL);
				if (window->firstElement->next == NULL)
					return 1;
				if (spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
					return 1;
			}
			else
				return 1;
		}
		else
			return 1;
	}
	if ((spMapGetByID(MAP_CHAT) && (spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER || spIsKeyboardPolled())) ||
		(spMapGetByID(MAP_MENU) && window->only_ok && selElem->type != -1))
	{
		spMapSetByID(MAP_CHAT,0);
		if (window->only_ok && selElem->type != -1)
			spMapSetByID(MAP_MENU,0);
		if (selElem->type == 1)
		{
			if (spIsKeyboardPolled())
			{
				if (window->feedback)
					window->feedback(window,selElem,WN_ACT_END_POLL);
				if (window->firstElement->next == NULL || spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
					return 1;
			}
			else
				return 1;
		}
		else
			return 1;
	}
	if ((spMapGetByID(MAP_SHOOT) || spMapGetByID(MAP_MENU)) &&
		(window->only_ok == 0 || selElem->type == -1))
	{
		spMapSetByID(MAP_SHOOT,0);
		spMapSetByID(MAP_MENU,0);
		switch (selElem->type)
		{
			case 1:
				if (spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
				{
					if (window->feedback)
						window->feedback(window,selElem,WN_ACT_END_POLL);
					if (window->firstElement->next == NULL)
						return 2;
				}
				else
				{
					if (spIsKeyboardPolled() && window->feedback)
						window->feedback(window,selElem,WN_ACT_END_POLL);
					return 2;
				}
				break;
			default:
				return 2;
		}
	}

	if (selElem->type == 1 &&
		!spIsKeyboardPolled() &&
		( spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER ||
		  window->firstElement->next == NULL ) &&
		  window->feedback)
		window->feedback(window,selElem,WN_ACT_START_POLL);

	if (spMapGetByID(MAP_CHAT) &&
		selElem->type == 1 &&
		!spIsKeyboardPolled() &&
		spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
	{
		spMapSetByID(MAP_CHAT,0);
		if (window->feedback)
			window->feedback(window,selElem,WN_ACT_START_POLL);
	}
	
	int i;
	if (spGetInput()->axis[0] < 0)
	{
		if (selElem->type == 0)
		{
			spGetInput()->axis[0] = 0;
			if (window->feedback)
				window->feedback(window,selElem,WN_ACT_LEFT);
			spGetInput()->touchscreen.pressed = 0;
		}
		else
		if (selElem->type == 2)
			for (i = 0; i < steps;i++)
				if (window->feedback)
					window->feedback(window,selElem,WN_ACT_LEFT);
	}
	if (spGetInput()->axis[0] > 0)
	{
		if (selElem->type == 0)
		{
			spGetInput()->axis[0] = 0;
			if (window->feedback)
				window->feedback(window,selElem,WN_ACT_RIGHT);
			spGetInput()->touchscreen.pressed = 0;
		}
		else
		if (selElem->type == 2)
			for (i = 0; i < steps;i++)
				if (window->feedback)
					window->feedback(window,selElem,WN_ACT_RIGHT);
	}
	if (window->feedback)
		window->feedback(window,selElem,WN_ACT_UPDATE);
	update_elem_width(selElem,window);
	update_window_width(window);
	return 0;
}

void set_recent_window(pWindow window)
{
	recent_window = window;
}

void window_resize(Uint16 w, Uint16 h)
{
	recent_window->resize(w,h);
	//Recalculating the window sizes.
	spDeleteSurface( recent_window->oldScreen );
	recent_window->oldScreen = spUniqueCopySurface( spGetWindowSurface() );
	update_window_width(recent_window);
	recent_window->height = recent_window->font->maxheight*4+(spGetSizeFactor()*4 >> SP_ACCURACY)*2-2*SMALL_HACK;
	pWindowElement elem = recent_window->firstElement;
	while (elem)
	{
		recent_window->height+=3*recent_window->font->maxheight/2-SMALL_HACK;
		elem = elem->next;
	}
	if (recent_window->show_selection)
		recent_window->height += (spGetSizeFactor()*16 >> SP_ACCURACY) + 2*recent_window->font->maxheight;
	if (recent_window->main_menu)
	{
		spDeleteSurface(logo);
		logo = spLoadSurfaceZoom( "./data/logo.png", spGetSizeFactor());
	}
	recent_window->sizeFactor = spGetSizeFactor();
}

int modal_window(pWindow window, void ( *resize )( Uint16 w, Uint16 h ))
{
	window_pause = 0;
	last_window_pause = 300;
	if (window->main_menu)
		logo = spLoadSurfaceZoom( "./data/logo.png", spGetSizeFactor());
	spUnlockRenderTarget();
	window->oldScreen = spUniqueCopySurface( spGetWindowSurface() );
	window->resize = resize;
	spLockRenderTarget();
	pWindow save_window = recent_window;
	recent_window = window;
	int res = spLoop(window_draw,window_calc,10,window_resize,NULL);
	if (spIsKeyboardPolled())
	{
		pWindowElement selElem = window->firstElement;
		int nr = 0;
		while (selElem)
		{
			if (nr == window->selection)
				break;
			nr++;
			selElem = selElem->next;
		}
		if (window->feedback)
			window->feedback(window,selElem,WN_ACT_END_POLL);
	}
	recent_window = save_window;
	spDeleteSurface( window->oldScreen );
	window->oldScreen = NULL;
	if (window->main_menu)
		spDeleteSurface( logo );
	if (window->show_selection)
		save_options();
	return res;
}

void delete_window(pWindow window)
{
	while (window->firstElement)
	{
		pWindowElement next = window->firstElement->next;
		free(window->firstElement);
		window->firstElement = next;
	}
	free(window);
}

void draw_message(void)
{
	pWindow save_window = recent_window;
	recent_window = mg_window;
	window_draw();
	recent_window = save_window;	
}

int set_message(spFontPointer font, char* caption)
{
	if (mg_window)
		delete_window(mg_window);
	mg_window = create_window(NULL,font,caption);
	mg_window->do_flip = 0;
	mg_window->zig_zag = 0;
}

int message_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption)
{
	pWindow window = create_window(NULL,font,caption);
	window->only_ok = 1;
	modal_window(window,resize);
	delete_window(window);
	return 0;
}

int text_box_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_START_POLL:
			spPollKeyboardInput(window->text_box_char,window->text_box_len,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	sprintf(elem->text,"%s",window->text_box_char);
	return 0;
}

pWindow create_text_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption, char* text,int len,int show_selection,int* sprite_count,int insult_button)
{
	pWindow window = create_window(text_box_feedback,font,caption);
	window->text_box_char = text;
	window->text_box_len = len;
	window->insult_button = insult_button;
	if (show_selection)
	{
		window->show_selection = show_selection;
		window->sprite_count = sprite_count;
		window->height += (spGetSizeFactor()*16 >> SP_ACCURACY) + 2*font->maxheight;
	}
	add_window_element(window,1,0);
	if (insult_button)
		window->width += spGetWindowSurface()->w/6;
	return window;
}

int text_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption, char* text,int len,int show_selection,int* sprite_count,int insult_button)
{
	pWindow window = create_text_box(font, resize, caption, text, len, show_selection, sprite_count, insult_button);
	int res = modal_window(window,resize);
	delete_window(window);
	return res;
}

int sprite_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption,int show_selection,int* sprite_count)
{
	pWindow window = create_window(text_box_feedback,font,caption);
	if (show_selection)
	{
		window->show_selection = show_selection;
		window->sprite_count = sprite_count;
		window->height += (spGetSizeFactor()*16 >> SP_ACCURACY) + 2*font->maxheight;
	}
	int res = modal_window(window,resize);
	delete_window(window);
	return res;
}
