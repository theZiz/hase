#include "lobbyList.h"
#include "client.h"
#include "level.h"
#include "window.h"
#include "options.h"

SDL_Surface* ll_surface;
SDL_Surface* ll_level = NULL;
spFontPointer ll_font;
int ll_counter;
pGame ll_game_list;
int ll_game_count;
int ll_scroll;
int ll_selected;
pPlayer ll_player_list = NULL;
spTextBlockPointer ll_block = NULL;
void ( *ll_resize )( Uint16 w, Uint16 h );
char ll_game_name[33] = "New game";
int ll_game_players = 4;
int ll_game_seconds = 45;
int ll_game_hares = 3;
spTextBlockPointer ll_chat_block = NULL;
Sint32 ll_chat_scroll;

pGame mom_game;

void update_ll_surface()
{
	spSelectRenderTarget(ll_surface);
	spClearTarget(LL_FG);
	int pos = 0;
	spFontDraw( 2                    , pos*ll_font->maxheight-ll_scroll/1024, 0, "Name", ll_font );
	spFontDraw( 2+ 7*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Mom Pl.", ll_font );
	spFontDraw( 2+10*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Max Pl.", ll_font );
	spFontDraw( 2+13*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Status", ll_font );
	pos++;
	spLine(2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, ll_surface->w-2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, 65535);
	pGame game = ll_game_list;
	mom_game = NULL;
	while (game)
	{
		if (pos-1 == ll_selected)
		{
			spRectangle(ll_surface->w/2, 2+(2*pos+1)*ll_font->maxheight/2-ll_scroll/1024,0, ll_surface->w-4,ll_font->maxheight,LL_BG);
			mom_game = game;
		}
		spFontDraw( 2                    , 2+pos*ll_font->maxheight-ll_scroll/1024, 0, game->name, ll_font );
		char buffer[16];
		sprintf(buffer,"%i",game->player_count);
		spFontDraw( 2+ 7*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		sprintf(buffer,"%i",game->max_player);
		spFontDraw( 2+10*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		switch (game->status)
		{
			case  1: sprintf(buffer,"Running"); break;
			case -1: sprintf(buffer,"Done"); break;
			default: sprintf(buffer,"Open");
		}
		spFontDraw( 2+13*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		game = game->next;
		pos++;
	}
	spSelectRenderTarget(spGetWindowSurface());
}

int ll_reload_now = 0;

void ll_draw(void)
{
	update_ll_surface();
	SDL_Surface* screen = spGetWindowSurface();
	spClearTarget(LL_BG);
	char buffer[256];
	sprintf(buffer,"Hase Lobby (Version %i)",CLIENT_VERSION);
	spFontDrawMiddle( screen->w/2, 0*ll_font->maxheight, 0, buffer, ll_font );
	
	sprintf(buffer,"%i Games on Server:\n",ll_game_count);
	spFontDrawMiddle( screen->w/3+2, 1*ll_font->maxheight, 0, buffer, ll_font );
	spBlitSurface(screen->w/3,ll_font->maxheight*2+ll_surface->h/2,0,ll_surface);
	
	spFontDrawMiddle(5*screen->w/6+4, 1*ll_font->maxheight, 0, "Preview", ll_font );
	spRectangle(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,screen->w/3-6,screen->w/3-6,LL_FG);
	if (ll_level)
		spBlitSurface(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,ll_level);

	int h = screen->h-(screen->w/3+4*ll_font->maxheight-4);
		
	if (get_channel() == NULL)
		spFontDrawMiddle(screen->w/3, screen->h-2*ll_font->maxheight-h/2, 0, "Connecting to IRC Server...", ll_font );
	else
	{
		spFontDrawMiddle(screen->w/3+2, 2*ll_font->maxheight+screen->w/3-6, 0, "[R]Chat [l]/[r]scroll", ll_font );
		spRectangle(screen->w/3, screen->h-1*ll_font->maxheight-h/2, 0,ll_surface->w,h,LL_FG);
		if (ll_chat_block)
			spFontDrawTextBlock(left,2, screen->h-1*ll_font->maxheight-h-1, 0,ll_chat_block,h+2,ll_chat_scroll,ll_font);
	}

	if (ll_game_count > 0 && mom_game)
	{
		sprintf(buffer,"Players of %s",mom_game->name);
		spFontDrawMiddle(5*screen->w/6+4, 2*ll_font->maxheight+screen->w/3-6, 0, buffer, ll_font );
		spRectangle(5*screen->w/6, screen->h-1*ll_font->maxheight-h/2, 0,screen->w/3-6,h,LL_FG);
		spFontDrawTextBlock(middle,4*screen->w/6+5, screen->h-1*ll_font->maxheight-h-1, 0,ll_block,h+2,0,ll_font);
	}
	
	if (ll_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-ll_font->maxheight, 0, "Reloading list...", ll_font );
		ll_reload_now = 2;
	}
	else
	{
		if (mom_game && mom_game->status == -1)
			spFontDraw( 2, screen->h-ll_font->maxheight, 0, "[o]Replay   [3]Create   [B]Back", ll_font );
		else
			spFontDraw( 2, screen->h-ll_font->maxheight, 0, "[o]Join   [3]Create   [B]Back", ll_font );
		sprintf(buffer,"Next update: %is",(10000-ll_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-ll_font->maxheight, 0, buffer, ll_font );
	}
	spFlip();
}

int ll_wait;
#define MAX_WAIT 300
#define MIN_WAIT 50

int create_game_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_LEFT:
			switch (elem->reference)
			{
				case 1:
					if (ll_game_players > 2)
						ll_game_players--;
					break;
				case 2:
					if (ll_game_seconds > 5)
						ll_game_seconds -= 5;
					break;
				case 3:
					if (ll_game_hares > 1)
						ll_game_hares--;
					break;
			}
			break;
		case WN_ACT_RIGHT:
			switch (elem->reference)
			{
				case 1:
					ll_game_players++;
					break;
				case 2:
					ll_game_seconds += 5;
					break;
				case 3:
					ll_game_hares++;
					break;
			}
			break;
		case WN_ACT_START_POLL:
			spPollKeyboardInput(ll_game_name,32,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	switch (elem->reference)
	{
		case 0: sprintf(elem->text,"Name: %s",ll_game_name); break;
		case 1: sprintf(elem->text,"Maximum players: %i",ll_game_players); break;
		case 2: sprintf(elem->text,"Seconds per turn: %i",ll_game_seconds); break;
		case 3: sprintf(elem->text,"Hares per player: %i",ll_game_hares); break;
	}
	return 0;
}

int ll_calc(Uint32 steps)
{
	try_to_join();
	int CHAT_LINES = (spGetWindowSurface()->h-(spGetWindowSurface()->w/3+4*ll_font->maxheight-4))/ll_font->maxheight;
	if (ll_chat_block)
	{
		if (ll_chat_block->line_count <= CHAT_LINES)
			ll_chat_scroll = 0;
		else
		{
			if (spGetInput()->button[MY_BUTTON_R])
			{
				spGetInput()->button[MY_BUTTON_R] = 0;
				ll_chat_scroll+=SP_ONE/(ll_chat_block->line_count-CHAT_LINES)+1;
				if (ll_chat_scroll > SP_ONE)
					ll_chat_scroll = SP_ONE;
			}
			if (spGetInput()->button[MY_BUTTON_L])
			{
				spGetInput()->button[MY_BUTTON_L] = 0;
				ll_chat_scroll-=SP_ONE/(ll_chat_block->line_count-CHAT_LINES)+1;
				if (ll_chat_scroll < 0)
					ll_chat_scroll = 0;
			}
		}
	}
	if (get_channel() && get_channel()->first_message)
	{
		char buffer[2048];
		if (get_channel()->last_read_message == NULL)
		{
			sprintf(buffer,"%s: %s",get_channel()->first_message->user,get_channel()->first_message->message);
			ll_chat_block = spCreateTextBlock(buffer,ll_surface->w-2,ll_font);
			get_channel()->last_read_message = get_channel()->first_message;
		}
		if (get_channel()->last_read_message)
			while (get_channel()->last_read_message->next)
			{
				spNetIRCMessagePointer next = get_channel()->last_read_message->next;
				sprintf(buffer,"%s: %s",next->user,next->message);
				spTextBlockPointer temp = spCreateTextBlock(buffer,ll_surface->w-2,ll_font);
				int lc = ll_chat_block->line_count + temp->line_count;
				spTextLinePointer copyLine = (spTextLinePointer)malloc(lc*sizeof(spTextLine));
				memcpy(copyLine,ll_chat_block->line,ll_chat_block->line_count*sizeof(spTextLine));
				memcpy(&copyLine[ll_chat_block->line_count],temp->line,temp->line_count*sizeof(spTextLine));
				free(ll_chat_block->line);
				ll_chat_block->line = copyLine;
				ll_chat_block->line_count = lc;
				free(temp);
				get_channel()->last_read_message = next;
				if (ll_chat_block->line_count > CHAT_LINES)
					ll_chat_scroll = SP_ONE;
			}
	}
	if (spGetInput()->button[MY_BUTTON_SELECT])
	{
		spGetInput()->button[MY_BUTTON_SELECT] = 0;
		return 1;
	}
	if (get_channel() && spGetInput()->button[MY_BUTTON_START])
	{
		spGetInput()->button[MY_BUTTON_START] = 0;
		char m[256] = "";
		if (text_box(ll_font,ll_resize,"Enter Message:",m,256,0,NULL,1) == 1)
			send_chat(NULL,m);
	}
	if (spGetInput()->button[MY_PRACTICE_3])
	{
		spGetInput()->button[MY_PRACTICE_3] = 0;
		int res = 1;
		while (res == 1)
		{
			pWindow window = create_window(create_game_feedback,ll_font,"Create game");
			add_window_element(window,1,0);
			add_window_element(window,0,1);
			add_window_element(window,0,2);
			add_window_element(window,0,3);
			res = modal_window(window,ll_resize);
			delete_window(window);
			if (res == 1 && ll_game_name[0] == 0)
				message_box(ll_font,ll_resize,"Please enter a game name");
			else
				break;
		}
		if (res == 1)
		{
			char buffer[512];
			pGame game = create_game(ll_game_name,ll_game_players,ll_game_seconds,create_level_string(buffer,1536,1536,3,3,3),0,ll_game_hares);
			start_lobby_game(ll_font,ll_resize,game);
			delete_game(game);
			ll_counter = 10000;			
		}
		ll_counter = 10000;
	}		
	if (spGetInput()->button[MY_PRACTICE_OK])
	{
		spGetInput()->button[MY_PRACTICE_OK] = 0;
		if (ll_game_count <= 0)
			message_box(ll_font,ll_resize,"No game to join!");
		else
		{
			pGame game = ll_game_list;
			int pos = 0;
			while (game)
			{
				if (pos == ll_selected)
					break;
				game = game->next;
				pos++;
			}
			if (game->status == 1)
				message_box(ll_font,ll_resize,"Game already started!");
			else
			if (game->status == -1) //Replay!
				hase(ll_resize,game,NULL);
			else
			if (game->player_count >= game->max_player)
				message_box(ll_font,ll_resize,"Game full!");
			else
			{
				start_lobby_game(ll_font,ll_resize,game);
				ll_counter = 10000;
			}
		}
	}
	if (ll_reload_now == 2)
	{
		int a = SDL_GetTicks();
		if (ll_reload())
			return 1;
		int b = SDL_GetTicks();
		ll_reload_now = 0;
		ll_counter = (a-b)/2; //<-black magic
		spResetLoop();
		steps = 0;
	}
	int step;
	for (step = 0; step < steps; step++)
	{
		ll_counter++;
		if (ll_wait > 0)
			ll_wait--;
		if (spGetInput()->axis[1] < 0)
		{
			if (ll_wait == -1)
			{
				
				ll_wait = MAX_WAIT;
				ll_selected--;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
			else
			if (ll_wait == 0)
			{
				ll_wait = MIN_WAIT;
				ll_selected--;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
		}
		else
		if (spGetInput()->axis[1] > 0)
		{
			if (ll_wait == -1)
			{	
				ll_wait = MAX_WAIT;
				ll_selected++;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
			else
			if (ll_wait == 0)
			{
				
				ll_wait = MIN_WAIT;
				ll_selected++;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
		}
		else
			ll_wait = -1;
	}
	if (ll_counter >= 10000)
		ll_reload_now = 1;
	int total_height = (ll_game_count+1)*ll_font->maxheight+2;
	if (ll_game_count > 1)
		ll_scroll = (total_height-ll_surface->h)*1024*ll_selected/(ll_game_count-1);
	if (ll_scroll/1024+ll_surface->h > total_height)
		ll_scroll = spMax(total_height-ll_surface->h,0)*1024;
	if (ll_scroll < 0)
		ll_scroll = 0;
	if (ll_selected < 0)
		ll_selected = 0;
	if (ll_selected >= ll_game_count)
		ll_selected = ll_game_count-1;
	if (ll_wait <0 && ll_selected >= 0 && ll_level == NULL)
	{
		pGame game = ll_game_list;
		int pos = 0;
		while (game)
		{
			if (pos == ll_selected)
				break;
			game = game->next;
			pos++;
		}
		if (game)
		{
			get_game(game,&ll_player_list);
			char temp[4096] = "";
			pPlayer player = ll_player_list;
			while (player)
			{
				add_to_string(temp,player->name);
				player = player->next;
				if (player)
					add_to_string(temp,", ");
			}
			ll_block = spCreateTextBlock(temp,spGetWindowSurface()->w/3-6,ll_font);
			ll_level = create_level(game->level_string,spGetWindowSurface()->w/3-6,spGetWindowSurface()->w/3-6,LL_BG);
			//texturize_level(ll_level,game->level_string);
		}
	}
	return 0;
}

int ll_reload()
{
	if (ll_level)
	{
		spDeleteSurface(ll_level);
		spDeleteTextBlock(ll_block);
		ll_level = NULL;
		ll_block = NULL;
	}
	if (connect_to_server())
	{
		message_box(ll_font,ll_resize,"No Connection to server!");
		return 1;
	}
	int info = server_info();
	if (info < 0)
	{
		message_box(ll_font,ll_resize,"The server is under repair. Please try again later!");
		return 1;
	}
	if (info != CLIENT_VERSION)
	{
		message_box(ll_font,ll_resize,"Your version is too old for\nonline games. Please update!");
		return 1;
	}
	start_irc_client(gop_username());
	ll_game_count = get_games(&ll_game_list);
	if (ll_game_count == 0)
		ll_selected = -1;
	return 0;
}


void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	ll_selected = 0;
	ll_font = font;
	ll_level = NULL;
	ll_block = NULL;
	ll_counter = 10000;//Instead reload
	ll_surface = spCreateSurface(2*spGetWindowSurface()->w/3-4,spGetWindowSurface()->w/3-6);
	ll_resize = resize;
	ll_reload_now = 0;
	ll_chat_block = NULL;
	spLoop(ll_draw,ll_calc,10,resize,NULL);
	spDeleteSurface(ll_surface);
	if (ll_level)
		spDeleteSurface(ll_level);
	if (ll_block)
		spDeleteTextBlock(ll_block);
	stop_irc_client();
}
