#include "lobbyList.h"
#include "client.h"
#include "level.h"
#include "window.h"
#include "options.h"
#include "hase.h"
#include <stdlib.h>

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
spTextBlockPointer ll_chat_block = NULL;
Sint32 ll_chat_scroll;

int use_chat = 0;
spTextBlockPointer lg_chat_block = NULL;
spFontPointer font_dark = NULL;
spFontPointer font = NULL;
spFontPointer help_font = NULL;
SDL_Surface* screen = NULL;

pGame mom_game;

tLobbyButton ll_button[ SP_MAPPING_MAX ];

#define LL_SURFACE_DIV 16
#define LL_MOM_PLAYER 11
#define LL_STATUS 13

void update_ll_surface()
{
	spSelectRenderTarget(ll_surface);
	spClearTarget(LL_FG);
	int pos = 0;
	spFontDraw( 2                    , pos*ll_font->maxheight-ll_scroll/1024, 0, "Name", ll_font );
	spFontDraw( 2+LL_MOM_PLAYER*ll_surface->w/LL_SURFACE_DIV, pos*ll_font->maxheight-ll_scroll/1024, 0, "Pl.", ll_font );
	spFontDraw( 2+LL_STATUS*ll_surface->w/LL_SURFACE_DIV, pos*ll_font->maxheight-ll_scroll/1024, 0, "Status", ll_font );
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
		spFontDraw( 2+LL_MOM_PLAYER*ll_surface->w/LL_SURFACE_DIV, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		switch (game->status)
		{
			case  1: sprintf(buffer,"Running"); break;
			case -1:
			{
				time_t rawtime;
				struct tm * ptm;
				time ( &rawtime );
				rawtime -= game->create_date;
				ptm = gmtime ( &rawtime );
				if (ptm->tm_yday)
					sprintf(buffer,"%id ago",ptm->tm_yday);
				else
				if (ptm->tm_hour)
					sprintf(buffer,"%ih ago",ptm->tm_hour);
				else
					sprintf(buffer,"%im ago",ptm->tm_min);
				break;
			}
			default: sprintf(buffer,"Open");
		}
		spFontDraw( 2+LL_STATUS*ll_surface->w/LL_SURFACE_DIV, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
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

	int i = SP_MAPPING_MAX;
	while (i --> 0 )
	{
		ll_button[i].x = -1;
		ll_button[i].y = -1;
		ll_button[i].w = -1;
		ll_button[i].h = -1;
	}

	int button_pos = spMax(spGetSizeFactor()>>15,1);

	sprintf(buffer,"%i Games on Server (Version %i):\n",ll_game_count,CLIENT_VERSION);
	spFontDrawMiddle( screen->w/3+2, 0*ll_font->maxheight-2, 0, buffer, ll_font );
	spBlitSurface(screen->w/3,ll_font->maxheight*1+ll_surface->h/2-1,0,ll_surface);

	spFontDrawMiddle(5*screen->w/6, 0*ll_font->maxheight-2, 0, "Preview & Players", ll_font );
	spRectangle(5*screen->w/6, ll_font->maxheight+screen->w/6-5, 0,screen->w/3-6,screen->w/3-6,LL_FG);
	if (ll_level)
		spBlitSurface(5*screen->w/6, 1*ll_font->maxheight+screen->w/6-5, 0,ll_level);

	int h = screen->h-(screen->w/3+3*ll_font->maxheight-6);

	if (use_chat == 0)
		spFontDrawMiddle(screen->w/2, screen->h-2*ll_font->maxheight-h/2, 0, "Chat deactivated", ll_font);
	else
	if (get_channel() == NULL)
		spFontDrawMiddle(screen->w/2, screen->h-2*ll_font->maxheight-h/2, 0, "Connecting to IRC Server...", ll_font );
	else
	{
		int rh = (h+3)/ll_font->maxheight*ll_font->maxheight-button_pos*5 + (spGetSizeFactor()>>17)*3;
		spRectangle(screen->w/2, screen->h-1*ll_font->maxheight-h/2-1, 0,screen->w-4,rh,LL_FG);
		if (ll_chat_block)
			spFontDrawTextBlock(left,2, screen->h-1*ll_font->maxheight-h-2+(h-rh)/2, 0,ll_chat_block,rh,ll_chat_scroll,ll_font);
		const int button_y = (
			screen->h-1*ll_font->maxheight-h/2-1 - rh/2 + //chat window top
			ll_font->maxheight+screen->w/6-5 + (screen->w/3-6) / 2 // preview window bottom
			) / 2 - ll_font->maxheight/2;
		lobby_draw_buttons( MIDDLE, screen->w/2, button_y, "{chat}Chat   {power_down}scroll up   {power_up}scroll down", ll_font, ll_button );
	}

	if (ll_game_count > 0 && mom_game && ll_block)
	{
		spFontDrawTextBlock(middle,4*screen->w/6+5, 1*ll_font->maxheight + 5, 0,ll_block,screen->w/3-6,0,ll_font);
	}

	if (ll_reload_now == 0)
	{
		if (mom_game && mom_game->status != 0)
			lobby_draw_buttons( LEFT, 2, screen->h-ll_font->maxheight-button_pos, "{jump}/{view}Show   {weapon}Create   {menu}Back", ll_font, ll_button );
		else
			lobby_draw_buttons( LEFT, 2, screen->h-ll_font->maxheight-button_pos, "{jump}Join   {weapon}Create   {view}Spectate   {menu}Back", ll_font, ll_button);
	}
	else
		lobby_draw_buttons( LEFT, 2, screen->h-ll_font->maxheight-button_pos, "{menu}Back", ll_font, ll_button);

	if (ll_reload_now == 1)
		ll_reload_now = 2;
	if (ll_reload_now)
		sprintf(buffer,"Reloading...");
	else
		sprintf(buffer,"Next update: %is",(10000-ll_counter)/1000);
	spFontDrawRight( screen->w-2, screen->h-ll_font->maxheight-2, 0, buffer, ll_font );

	spFlip();
}

int ll_wait;
#define MAX_WAIT 300
#define MIN_WAIT 50

SDL_Thread* ll_thread = NULL;

int create_game_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_START_POLL:
			spPollKeyboardInput(ll_game_name,32,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	sprintf(elem->text,"Name: %s",ll_game_name);
	return 0;
}

int ll_calc(Uint32 steps)
{
	if ( spGetInput()->touchscreen.pressed )
	{
		int mx = spGetInput()->touchscreen.x;
		int my = spGetInput()->touchscreen.y;
		int i = SP_MAPPING_MAX;
		while (i --> 0 )
		{
			if ( ll_button[i].x + ll_button[i].w >= mx &&
				ll_button[i].x <= mx &&
				ll_button[i].y + ll_button[i].h >= my &&
				ll_button[i].y <= my )
			{
				spMapSetByID( i, 1 );
				spGetInput()->touchscreen.pressed = 0;
				break;
			}
		}
	}
	try_to_join();
	int CHAT_LINES = (spGetWindowSurface()->h-(spGetWindowSurface()->w/3+4*ll_font->maxheight-4))/ll_font->maxheight;
	if (ll_chat_block)
	{
		if (ll_chat_block->line_count <= CHAT_LINES)
			ll_chat_scroll = 0;
		else
		{
			if (spMapGetByID(MAP_POWER_UP))
			{
				spMapSetByID(MAP_POWER_UP,0);
				ll_chat_scroll+=SP_ONE/(ll_chat_block->line_count-CHAT_LINES)+1;
				if (ll_chat_scroll > SP_ONE)
					ll_chat_scroll = SP_ONE;
			}
			if (spMapGetByID(MAP_POWER_DN))
			{
				spMapSetByID(MAP_POWER_DN,0);
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
			if (strcmp(get_channel()->first_message->ctcp,"ACTION") == 0)
				sprintf(buffer,"*** %s %s",get_channel()->first_message->user,get_channel()->first_message->message);
			else
				sprintf(buffer,"%s: %s",get_channel()->first_message->user,get_channel()->first_message->message);
			ll_chat_block = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,ll_font);
			get_channel()->last_read_message = get_channel()->first_message;
		}
		if (get_channel()->last_read_message)
			while (get_channel()->last_read_message->next)
			{
				spNetIRCMessagePointer next = get_channel()->last_read_message->next;

				log_message(next->user,next->message);

				if (strcmp(next->ctcp,"ACTION") == 0)
					sprintf(buffer,"*** %s %s",next->user,next->message);
				else
					sprintf(buffer,"%s: %s",next->user,next->message);
				spTextBlockPointer temp = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,ll_font);
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
	if (spMapGetByID(MAP_MENU))
	{
		spMapSetByID(MAP_MENU,0);
		return 1;
	}
	if (get_channel() && spMapGetByID(MAP_CHAT))
	{
		spMapSetByID(MAP_CHAT,0);
		char m[256] = "";
		if (text_box(ll_font,ll_resize,"Enter Message:",m,256,0,NULL,1) == 1 && m[0])
			send_chat(NULL,m);
	}
	if (spMapGetByID(MAP_WEAPON) && ll_reload_now == 0)
	{
		spMapSetByID(MAP_WEAPON,0);
		int res = 1;
		while (res == 1)
		{
			pWindow window = create_window(create_game_feedback,ll_font,"Create game");
			add_window_element(window,1,0);
			res = modal_window(window,ll_resize);
			delete_window(window);
			if (res == 1 && ll_game_name[0] == 0)
				message_box(ll_font,ll_resize,"Please enter a game name");
			else
				break;
		}
		if (res == 1)
		{
			if (game_options(gop_game_options_ptr(),gop_game_seconds_ptr(),gop_game_hares_ptr(),ll_font,ll_resize) == 1)
			{
				char buffer[512];
				pGame game = create_game(ll_game_name,*gop_game_options_ptr(),*gop_game_seconds_ptr(),create_level_string(buffer,1536,1536,3,3,3),0,*gop_game_hares_ptr());
				start_lobby_game(ll_font,ll_resize,game,0);
				delete_game(game);
			}
		}
		ll_counter = 10000;
	}
	if (spMapGetByID(MAP_JUMP) && ll_reload_now == 0)
	{
		spMapSetByID(MAP_JUMP,0);
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
			start_lobby_game(ll_font,ll_resize,game,game->status != 0);
			ll_counter = 10000;
		}
	}
	if (spMapGetByID(MAP_VIEW) && ll_reload_now == 0)
	{
		spMapSetByID(MAP_VIEW,0);
		if (ll_game_count <= 0)
			message_box(ll_font,ll_resize,"No game to spectate!");
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
			start_lobby_game(ll_font,ll_resize,game,1);
			ll_counter = 10000;
		}
	}
	if (ll_reload_now == 2)
	{
		ll_reload_now = 3;
		ll_thread = SDL_CreateThread(ll_reload,NULL);
	}
	if (ll_reload_now == 4)
	{
		int result;
		SDL_WaitThread(ll_thread,&result);
		ll_thread = NULL;
		ll_reload_now = 0;
		ll_counter = 0;
		switch (result)
		{
			case 1:
				message_box(ll_font,ll_resize,"No Connection to server!");
				return 1;
			case 2:
				message_box(ll_font,ll_resize,"The server is under repair. Please try again later!");
				return 1;
			case 3:
				message_box(ll_font,ll_resize,"Your version is too old for\nonline games. Please update!");
				return 1;
		}
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
	if (ll_counter >= 10000 && ll_reload_now == 0)
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

int ll_reload(void* dummy)
{
	if (connect_to_server())
	{
		ll_reload_now = 4;
		return 1;
	}
	int info = server_info();
	if (info < 0)
	{
		ll_reload_now = 4;
		return 2;
	}
	if (info != CLIENT_VERSION)
	{
		ll_reload_now = 4;
		return 3;
	}
	if (use_chat)
		start_irc_client(gop_username());
	ll_game_count = get_games(&ll_game_list);
	if (ll_game_count == 0)
		ll_selected = -1;
	if (ll_level)
	{
		SDL_Surface* ll_level_cp = ll_level;
		spTextBlockPointer ll_block_cp = ll_block;
		ll_level = NULL;
		ll_block = NULL;
		spDeleteSurface(ll_level_cp);
		spDeleteTextBlock(ll_block_cp);
	}
	ll_reload_now = 4;
	return 0;
}


void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), int start_chat)
{
	int i = SP_MAPPING_MAX;
	while (i --> 0 )
	{
		ll_button[i].x = -1;
		ll_button[i].y = -1;
		ll_button[i].w = -1;
		ll_button[i].h = -1;
	}
	use_chat = start_chat;
	char time_buffer[128];
    time_t t = time(NULL);
    struct tm *ti = gmtime(&t);
    strftime(time_buffer,2048,"%c",ti);

	log_message("Enter lobby",time_buffer);

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
	if (ll_thread)
	{
		SDL_KillThread(ll_thread);
		ll_thread = NULL;
	}
	spDeleteSurface(ll_surface);
	if (ll_level)
		spDeleteSurface(ll_level);
	if (ll_block)
		spDeleteTextBlock(ll_block);
	stop_irc_client();
}
