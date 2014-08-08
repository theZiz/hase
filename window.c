#include "window.h"
#include "lobbyList.h"

pWindow mg_window = NULL;

void update_window_width(pWindow window)
{
	pWindowElement elem = window->firstElement;
	while (elem)
	{
		window->width = spMax(window->width,elem->width+(spGetSizeFactor()*4 >> SP_ACCURACY)*2);
		elem = elem->next;
	}
	window->width = spMin(window->width,spGetWindowSurface()->w-(spGetSizeFactor()*4 >> SP_ACCURACY)*2);
}

pWindow create_window(int ( *feedback )( pWindowElement elem, int action ),spFontPointer font,char* title)
{
	pWindow window = (pWindow)malloc(sizeof(tWindow));
	window->height = font->maxheight*4+(spGetSizeFactor()*4 >> SP_ACCURACY)*2;
	window->font = font;
	window->feedback = feedback;
	window->selection = 0;
	window->firstElement = NULL;
	sprintf(window->title,"%s",title);
	window->width = spMax(spGetWindowSurface()->w/2,spFontWidth(title,font)+(spGetSizeFactor()*4 >> SP_ACCURACY)*2);
	update_window_width(window);
	window->do_flip = 1;
	window->main_menu = 0;
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
	window->feedback(elem,WN_ACT_UPDATE);
	update_elem_width(elem,window);
	update_window_width(window);
	window->height+=2*window->font->maxheight;
	return elem;
}

pWindow recent_window = NULL;

void window_draw(void)
{
	SDL_Surface* screen = spGetWindowSurface();
	spSetAlphaPattern4x4(196,0);
	spRectangle(screen->w/2,screen->h/2,0,screen->w,screen->h,LL_BG);
	spDeactivatePattern();

	if (spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
		spBlitSurface(screen->w/2,screen->h-spGetVirtualKeyboard()->h/2,0,spGetVirtualKeyboard());

	char buffer[256];
	pWindow window = recent_window;
	int meow = spGetSizeFactor()*4 >> SP_ACCURACY;
	spRectangle(screen->w/2,screen->h/2,0,window->width-2*meow,window->height-2*meow,LL_BG);
	spRectangleBorder(screen->w/2,screen->h/2,0,window->width,window->height,meow,meow,LL_FG);
	int y = (screen->h - window->height) / 2 + meow + window->font->maxheight/2;
	spFontDrawMiddle( screen->w/2, y , 0, window->title, window->font );
	pWindowElement elem = window->firstElement;
	pWindowElement selElem = NULL;
	int nr = 0;
	while (elem)
	{
		y+=window->font->maxheight*2;
		if (nr == window->selection)
		{
			if (window->firstElement->next) //more than one element
				spRectangle( screen->w/2, y+window->font->maxheight/2, 0, window->width-2*meow,window->font->maxheight,LL_FG);
			selElem = elem;
			if (elem->type == 1 && spIsKeyboardPolled())
			{
				int x = (screen->w+spFontWidth(elem->text,window->font))/2;
				if ((SDL_GetTicks() >> 9) & 1)
					spLine(x, y, 0, x, y+window->font->maxheight, 0,65535);
			}
		}
		spFontDrawMiddle( screen->w/2, y, 0, elem->text, window->font );
		nr++;
		elem = elem->next;
	}
	y = (screen->h + window->height) / 2 - meow - 3*window->font->maxheight/2;
	if (selElem)
	{
		switch (selElem->type)
		{
			case 0:
				spFontDrawMiddle( screen->w/2,y, 0, SP_PAD_NAME": Change  [o]Okay  [c]Cancel", window->font );
				break;
			case 1:
				if (spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
				{
					if (spIsKeyboardPolled())
					{
						if (window->firstElement->next)
							spFontDrawMiddle( screen->w/2,y, 0, "[3]Enter letter  [o]/[c]Back", window->font );
						else
							spFontDrawMiddle( screen->w/2,y, 0, "[3]Enter letter  [o]Okay  [c]Cancel", window->font );
					}
					else
						spFontDrawMiddle( screen->w/2,y, 0, "[3]Change  [o]Okay  [c]Cancel", window->font );
				}
				else
					spFontDrawMiddle( screen->w/2,y, 0, "Keyboard: Change  [o]Okay  [c]Cancel", window->font );
				break;
			case -1:
				if (window->main_menu)
					spFontDrawMiddle( screen->w/2,y, 0, "[o]Select  [c]Exit", window->font );
				else
					spFontDrawMiddle( screen->w/2,y, 0, "[o]Okay  [c]Cancel", window->font );
				break;
			case -2:
				spFontDrawMiddle( screen->w/2,y, 0, "[o]Okay", window->font );
				break;
		}
	}
	else
	if (window->do_flip)
		spFontDrawMiddle( screen->w/2,y, 0, "[o]Okay", window->font );
	if (window->do_flip)
		spFlip();
}

int window_calc(Uint32 steps)
{
	pWindow window = recent_window;
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
		if (spGetInput()->button[MY_PRACTICE_OK])
		{
			spGetInput()->button[MY_PRACTICE_OK] = 0;
			return 1;
		}
		return 0;
	}
	if (selElem->type != 1 ||
		!spIsKeyboardPolled() ||
		spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
	{
		if (spGetInput()->axis[1] < 0 && befElem)
		{
			spGetInput()->axis[1] = 0;
			if (selElem->type == 1 &&
				spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
				window->feedback(selElem,WN_ACT_END_POLL);
			window->selection--;
			selElem = befElem;
		}
		if (spGetInput()->axis[1] > 0 && selElem->next)
		{
			spGetInput()->axis[1] = 0;
			if (selElem->type == 1 &&
				spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER)
				window->feedback(selElem,WN_ACT_END_POLL);
			window->selection++;
			selElem = selElem->next;
		}
	}
	if (spGetInput()->button[MY_PRACTICE_OK])
	{
		spGetInput()->button[MY_PRACTICE_OK] = 0;
		switch (selElem->type)
		{
			case 1:
				if (spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
				{
					window->feedback(selElem,WN_ACT_END_POLL);
					if (window->firstElement->next == NULL)
						return 1;
				}
				else
					return 1;
				break;
			default:
				return 1;
		}
	}
	if (spGetInput()->button[MY_PRACTICE_CANCEL])
	{
		spGetInput()->button[MY_PRACTICE_CANCEL] = 0;
		switch (selElem->type)
		{
			case 1:
				if (spIsKeyboardPolled() && spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
				{
					window->feedback(selElem,WN_ACT_END_POLL);
					if (window->firstElement->next == NULL)
						return 2;
				}
				else
					return 2;
				break;
			default:
				return 2;
		}
	}

	if (selElem->type == 1 &&
		!spIsKeyboardPolled() &&
		( spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_NEVER ||
		  window->firstElement->next == NULL ) )
		window->feedback(selElem,WN_ACT_START_POLL);

	if (spGetInput()->button[MY_PRACTICE_3] &&
		selElem->type == 1 &&
		!spIsKeyboardPolled() &&
		spGetVirtualKeyboardState() == SP_VIRTUAL_KEYBOARD_ALWAYS)
	{
		spGetInput()->button[MY_PRACTICE_3] = 0;
		window->feedback(selElem,WN_ACT_START_POLL);
	}

	if (selElem->type == 0 && spGetInput()->axis[0] < 0)
	{
		spGetInput()->axis[0] = 0;
		window->feedback(selElem,WN_ACT_LEFT);
	}
	if (selElem->type == 0 && spGetInput()->axis[0] > 0)
	{
		spGetInput()->axis[0] = 0;
		window->feedback(selElem,WN_ACT_RIGHT);
	}
	window->feedback(selElem,WN_ACT_UPDATE);
	update_elem_width(selElem,window);
	update_window_width(window);
	return 0;
}

int modal_window(pWindow window, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow save_window = recent_window;
	recent_window = window;
	int res = spLoop(window_draw,window_calc,10,resize,NULL);
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
		window->feedback(selElem,WN_ACT_END_POLL);
	}
	recent_window = save_window;
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
}

void message_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption)
{
	pWindow window = create_window(NULL,font,caption);
	modal_window(window,resize);
	delete_window(window);
}

char* text_box_char = NULL;
int text_box_len = 0;

int text_box_feedback( pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_START_POLL:
			spPollKeyboardInput(text_box_char,text_box_len,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	sprintf(elem->text,"%s",text_box_char);
	return 0;
}

int text_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption, char* text,int len)
{
	char* save_char = text_box_char;
	int save_len = text_box_len;
	text_box_char = text;
	text_box_len = len;
	pWindow window = create_window(text_box_feedback,font,caption);
	add_window_element(window,1,0);
	int res = modal_window(window,resize);
	delete_window(window);
	text_box_char = save_char;
	text_box_len = save_len;
	return res;
}
