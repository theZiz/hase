#include "lobbyList.h"
#include "client.h"
#include "level.h"
SDL_Surface* ll_surface;
SDL_Surface* ll_level = NULL;
spFontPointer ll_font;
int ll_counter;
pGame ll_game_list;
int ll_game_count;
int ll_offline;
int ll_scroll;
int ll_selected;
pPlayer ll_player_list;
spTextBlockPointer ll_block = NULL;

void update_ll_surface()
{
	spSelectRenderTarget(ll_surface);
	spClearTarget(LL_FG);
	int pos = 0;
	spFontDraw( 2                    , pos*ll_font->maxheight-ll_scroll/1024, 0, "Name", ll_font );
	spFontDraw( 2+ 7*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Mom. Pl.", ll_font );
	spFontDraw( 2+10*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Max. Pl.", ll_font );
	spFontDraw( 2+13*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Status", ll_font );
	pos++;
	spLine(2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, ll_surface->w-2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, 65535);
	pGame game = ll_game_list;
	while (game)
	{
		if (pos-1 == ll_selected)
			spRectangle(ll_surface->w/2, 2+(2*pos+1)*ll_font->maxheight/2-ll_scroll/1024,0, ll_surface->w-4,ll_font->maxheight,LL_BG);
		spFontDraw( 2                    , 2+pos*ll_font->maxheight-ll_scroll/1024, 0, game->name, ll_font );
		char buffer[16];
		sprintf(buffer,"%i",game->player_count);
		spFontDraw( 2+ 7*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		sprintf(buffer,"%i",game->max_player);
		spFontDraw( 2+10*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		switch (game->status)
		{
			case  1: sprintf(buffer,"Running"); break;
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
	if (ll_offline)
		spFontDrawMiddle( screen->w/2, 0*ll_font->maxheight, 0, "Hase Lobby (No internet cnnection)", ll_font );
	else
		spFontDrawMiddle( screen->w/2, 0*ll_font->maxheight, 0, "Hase Lobby", ll_font );
	
	sprintf(buffer,"%i Games on Server:\n",ll_game_count);
	spFontDrawMiddle( screen->w/3+2, 1*ll_font->maxheight, 0, buffer, ll_font );
	spBlitSurface(screen->w/3,screen->h/2+ll_font->maxheight/2,0,ll_surface);
	spFontDrawMiddle(5*screen->w/6+4, 1*ll_font->maxheight, 0, "Preview", ll_font );
	spRectangle(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,screen->w/3-6,screen->w/3-6,LL_FG);
	if (ll_level)
		spBlitSurface(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,ll_level);
	spFontDrawMiddle(5*screen->w/6+4, 2*ll_font->maxheight+screen->w/3-6, 0, "Players", ll_font );
	int h = screen->h-(screen->w/3+4*ll_font->maxheight-4);
	spRectangle(5*screen->w/6, screen->h-1*ll_font->maxheight-h/2, 0,screen->w/3-6,h,LL_FG);
	spFontDrawTextBlock(middle,4*screen->w/6+6, screen->h-1*ll_font->maxheight-h, 0,
		ll_block,h,0,ll_font);
	if (ll_reload_now)
		spFontDrawMiddle( screen->w/2, screen->h-ll_font->maxheight, 0, "Reloading list...", ll_font );
	else
	{
		spFontDraw( 2, screen->h-ll_font->maxheight, 0, "Lorem ipsum", ll_font );
		sprintf(buffer,"Next update: %is",(10000-ll_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-ll_font->maxheight, 0, buffer, ll_font );
	}
	spFlip();
}

int ll_wait;
#define MAX_WAIT 300
#define MIN_WAIT 50

int ll_calc(Uint32 steps)
{
	if (spGetInput()->button[SP_BUTTON_SELECT_NOWASD])
		return 1;
	if (ll_reload_now)
	{
		int a = SDL_GetTicks();
		ll_reload();
		int b = SDL_GetTicks();
		ll_reload_now = 0;
		ll_counter = a-b;
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

void ll_reload()
{
	if (ll_level)
	{
		spDeleteSurface(ll_level);
		spDeleteTextBlock(ll_block);
		ll_level = NULL;
		ll_block = NULL;
	}
	ll_offline = connect_to_server();
	if (ll_offline == 0)
		ll_offline = server_info() == 0;
	if (!ll_offline)
		ll_game_count = get_games(&ll_game_list);
	else
	{
		ll_game_list = NULL;
		ll_game_count = 0;
	}
	if (ll_game_count == 0)
		ll_selected = -1;	
}


void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	ll_selected = 0;
	ll_reload();
	ll_font = font;
	ll_counter = 0;
	ll_surface = spCreateSurface(2*spGetWindowSurface()->w/3-4,spGetWindowSurface()->h-3*font->maxheight);
	spLoop(ll_draw,ll_calc,10,resize,NULL);
	spDeleteSurface(ll_surface);
	if (ll_level)
		spDeleteSurface(ll_level);
	if (ll_block)
		spDeleteTextBlock(ll_block);
}
