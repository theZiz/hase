#include "lobbyGame.h"
#include "level.h"

spFontPointer lg_font;
void ( *lg_resize )( Uint16 w, Uint16 h );
int lg_counter;
char lg_name[32] = "";
int lg_local;
pGame lg_game;
int lg_reload_now = 0;
SDL_Surface* lg_level = NULL;
pPlayer lg_player_list = NULL;
pPlayer lg_player;
spTextBlockPointer lg_block = NULL;
char lg_level_string[512];

void lg_draw(void)
{
	SDL_Surface* screen = spGetWindowSurface();
	spClearTarget(LL_BG);
	char buffer[256];
	if (lg_game->local)
		sprintf(buffer, "%s (Local)",lg_game->name);
	else
		sprintf(buffer, "%s (Internet)",lg_game->name);
	spFontDrawMiddle( screen->w/2, 0*lg_font->maxheight, 0, buffer, lg_font );
	
	spFontDrawMiddle(2+(screen->h-3*lg_font->maxheight)/2, 1*lg_font->maxheight, 0, "Preview", lg_font );
	spRectangle  (2+(screen->h-3*lg_font->maxheight)/2, 2*lg_font->maxheight+(screen->h-3*lg_font->maxheight)/2, 0,screen->h-3*lg_font->maxheight,screen->h-3*lg_font->maxheight,LL_FG);
	if (lg_level)
		spBlitSurface(2+(screen->h-3*lg_font->maxheight)/2, 2*lg_font->maxheight+(screen->h-3*lg_font->maxheight)/2, 0,lg_level);
	int w = screen->w-8-(screen->h-3*lg_font->maxheight);
	spFontDrawMiddle(screen->w-2-w/2, 1*lg_font->maxheight, 0, "Game Info", lg_font );
	sprintf(buffer,"Seconds per turn: %i",lg_game->seconds_per_turn);
	spFontDraw(screen->w-w+2, 2*lg_font->maxheight, 0, buffer, lg_font );
	sprintf(buffer,"Maximum players: %i",lg_game->max_player);
	spFontDraw(screen->w-w+2, 3*lg_font->maxheight, 0, buffer, lg_font );
	spFontDraw(screen->w-w+2, 4*lg_font->maxheight, 0, "Players:", lg_font );
	int h = screen->h-8*lg_font->maxheight;
	spRectangle(screen->w-2-w/2, 5*lg_font->maxheight+(screen->h-8*lg_font->maxheight)/2, 0,w,h,LL_FG);
	if (lg_block)
		spFontDrawTextBlock(middle,screen->w-w+2, 5*lg_font->maxheight, 0,
			lg_block,h,0,lg_font);
	if (lg_game->admin_pw == 0)
	{
		spFontDrawMiddle(screen->w-2-w/2, screen->h-3*lg_font->maxheight, 0, "The game master will", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, screen->h-2*lg_font->maxheight, 0, "start the game soon™.", lg_font );
	}
	else
	{
		spFontDrawMiddle(screen->w-2-w/2, screen->h-3*lg_font->maxheight, 0, "[d] New level     [a] Start game", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, screen->h-2*lg_font->maxheight, 0, "[w] Add AI     [s] Remove all AI", lg_font );
	}
	if (lg_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-lg_font->maxheight, 0, "Reloading list...", lg_font );
		lg_reload_now = 2;
	}
	else
	{
		if (lg_game->admin_pw == 0)
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[R] or [B] Leave game", lg_font );
		else
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[R] or [B] Leave and close game", lg_font );
		sprintf(buffer,"Next update: %is",(10000-lg_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-lg_font->maxheight, 0, buffer, lg_font );
	}
	spFlip();
}

pPlayer lg_ai_list = NULL;

int lg_calc(Uint32 steps)
{
	if (spGetInput()->button[SP_BUTTON_SELECT])
	{
		spGetInput()->button[SP_BUTTON_SELECT] = 0;
		return 1;
	}
	if (spGetInput()->button[SP_BUTTON_LEFT] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_LEFT] = 0;
		if (lg_game->player_count < 2)
			message(lg_font,lg_resize,"At least two players are needed!",1,NULL);
		else
		{
			printf("Running game\n");
			set_status(lg_game,1);
			hase(lg_resize,lg_game);
			return 2;
		}
	}
	if (spGetInput()->button[SP_BUTTON_RIGHT] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_RIGHT] = 0;
		create_level_string(lg_game->level_string,1536,1536,3,3,3);
		spDeleteSurface(lg_level);
		lg_level = create_level(lg_game->level_string,spGetWindowSurface()->h-3*lg_font->maxheight,spGetWindowSurface()->h-3*lg_font->maxheight,LL_BG);
		sprintf(lg_level_string,"%s",lg_game->level_string);
		set_level(lg_game,lg_level_string);
	}
	if (spGetInput()->button[SP_BUTTON_DOWN] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_DOWN] = 0;
		while (lg_ai_list)
		{
			pPlayer next = lg_ai_list->next;
			leave_game(lg_ai_list);
			lg_ai_list = next;
		}
		lg_counter = 10000;
	}
	if (spGetInput()->button[SP_BUTTON_UP] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_UP] = 0;
		pPlayer ai = join_game(lg_game,"AI",1);
		if (ai)
		{
			ai->next = lg_ai_list;
			lg_ai_list = ai;
			lg_counter = 10000;
		}
	}
	if (lg_reload_now == 2)
	{
		int a = SDL_GetTicks();
		int res = lg_reload();
		int b = SDL_GetTicks();
		lg_reload_now = 0;
		lg_counter = a-b;
		if (res == -1)
			return -1; //stopped
		if (res == 1)
			return 2; //started
		
	}
	int step;
	for (step = 0; step < steps; step++)
	{
		lg_counter++;
	}
	if (lg_counter >= 10000)
		lg_reload_now = 1;
	return 0;
}

int lg_reload()
{
	get_game(lg_game,&lg_player_list);
	char temp[4096] = "";
	pPlayer player = lg_player_list;
	while (player)
	{
		add_to_string(temp,player->name);
		player = player->next;
		if (player)
			add_to_string(temp,", ");
	}
	if (lg_block)
		spDeleteTextBlock(lg_block);
	lg_block = spCreateTextBlock(temp,spGetWindowSurface()->w-8-(spGetWindowSurface()->h-3*lg_font->maxheight),lg_font);
	if (lg_level == NULL || strcmp(lg_level_string,lg_game->level_string))
	{
		spDeleteSurface(lg_level);
		lg_level = create_level(lg_game->level_string,spGetWindowSurface()->h-3*lg_font->maxheight,spGetWindowSurface()->h-3*lg_font->maxheight,LL_BG);
		sprintf(lg_level_string,"%s",lg_game->level_string);
	}

	return lg_game->status;
}

void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), pGame game)
{
	lg_game = game;
	lg_font = font;
	lg_counter = 10000; //instead reload
	lg_local = 0;
	lg_reload_now = 0;
	lg_ai_list = NULL;
	lg_resize = resize;
	spPollKeyboardInput(lg_name,32,SP_BUTTON_UP_MASK);
	if (message(font,resize,"Enter player name:",2,lg_name) == 1)
	{
		spStopKeyboardInput();
		if (lg_name[0] == 0)
		{
			message(font,resize,"No name entered...",1,NULL);
			return;
		}
		if ((lg_player = join_game(game,lg_name,0)) == NULL)
		{
			message(font,resize,"Game full...",1,NULL);
			return;
		}
		lg_level = NULL;
		lg_level_string[0] = 0;
		lg_block = NULL;
		int res = spLoop(lg_draw,lg_calc,10,resize,NULL);
		if (lg_block)
			spDeleteTextBlock(lg_block);
		if (res == -1)
			message(font,resize,"Game was closed...",1,NULL);
		else
		if (res == 2)
		{
			printf("Starting game!\n");
		}
		leave_game(lg_player);
		spDeleteSurface(lg_level);
		while (lg_ai_list)
		{
			pPlayer next = lg_ai_list->next;
			leave_game(lg_ai_list);
			lg_ai_list = next;
		}
			
	}
	else
		spStopKeyboardInput();
}

