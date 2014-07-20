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
pPlayer lg_last_player;
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
	sprintf(buffer,"Sec. per turn: %i",lg_game->seconds_per_turn);
	spFontDraw(screen->w-w, 2*lg_font->maxheight, 0, buffer, lg_font );
	sprintf(buffer,"Max. players: %i",lg_game->max_player);
	spFontDraw(screen->w-w, 3*lg_font->maxheight, 0, buffer, lg_font );
	spFontDraw(screen->w-w, 4*lg_font->maxheight, 0, "Players:", lg_font );
	int h = screen->h-9*lg_font->maxheight;
	spRectangle(screen->w-2-w/2, 5*lg_font->maxheight+(screen->h-9*lg_font->maxheight)/2, 0,w,h,LL_FG);
	if (lg_block)
		spFontDrawTextBlock(middle,screen->w-w+2, 5*lg_font->maxheight, 0,
			lg_block,h,0,lg_font);
	spFontDrawMiddle(screen->w-2-w/2, screen->h-4*lg_font->maxheight, 0, "[w]Add pl.  [s]Rem. pl.", lg_font );
	if (lg_game->admin_pw == 0)
	{
		spFontDrawMiddle(screen->w-2-w/2, screen->h-3*lg_font->maxheight, 0, "The game master will", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, screen->h-2*lg_font->maxheight, 0, "start the game soon™.", lg_font );
	}
	else
	{
		spFontDrawMiddle(screen->w-2-w/2, screen->h-3*lg_font->maxheight, 0, "[d]New level  [a]Start", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, screen->h-2*lg_font->maxheight, 0, "[q]Add AI  [e]Rem. all AI", lg_font );
	}
	if (lg_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-lg_font->maxheight, 0, "Reloading list...", lg_font );
		lg_reload_now = 2;
	}
	else
	{
		if (lg_game->admin_pw == 0)
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[B]Leave game", lg_font );
		else
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[B]Leave and close game", lg_font );
		sprintf(buffer,"Next update: %is",(10000-lg_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-lg_font->maxheight, 0, buffer, lg_font );
	}
	spFlip();
}

pPlayer lg_ai_list = NULL;

char* lg_get_name(char* buffer)
{
	int i = rand()%50;
	switch (i)
	{
		//Germany
		case  0: sprintf(buffer,"Elias"); break;
		case  1: sprintf(buffer,"Alexander"); break;
		case  2: sprintf(buffer,"Daniel"); break;
		case  3: sprintf(buffer,"Lucas"); break;
		case  4: sprintf(buffer,"Michael"); break;
		case  5: sprintf(buffer,"Julia"); break;
		case  6: sprintf(buffer,"Laura"); break;
		case  7: sprintf(buffer,"Anna"); break;
		case  8: sprintf(buffer,"Lea"); break;
		case  9: sprintf(buffer,"Emma"); break;
		//Sweden
		case 10: sprintf(buffer,"William"); break;
		case 11: sprintf(buffer,"Oscar"); break;
		case 12: sprintf(buffer,"Hugo"); break;
		case 13: sprintf(buffer,"Liam"); break;
		case 14: sprintf(buffer,"Charlie"); break;
		case 15: sprintf(buffer,"Alice"); break;
		case 16: sprintf(buffer,"Elsa"); break;
		case 17: sprintf(buffer,"Ella"); break;
		case 18: sprintf(buffer,"Maja"); break;
		case 19: sprintf(buffer,"Ebba"); break;
		//USA
		case 20: sprintf(buffer,"Jacob"); break;
		case 21: sprintf(buffer,"Mason"); break;
		case 22: sprintf(buffer,"Ethan"); break;
		case 23: sprintf(buffer,"Jayden"); break;
		case 24: sprintf(buffer,"Michael"); break;
		case 25: sprintf(buffer,"Sophia"); break;
		case 26: sprintf(buffer,"Isabella"); break;
		case 27: sprintf(buffer,"Olivia"); break;
		case 28: sprintf(buffer,"Ava"); break;
		case 29: sprintf(buffer,"Emily"); break;
		//France
		case 30: sprintf(buffer,"Arthur"); break;
		case 31: sprintf(buffer,"Jules"); break;
		case 32: sprintf(buffer,"Nolan"); break;
		case 33: sprintf(buffer,"Louis"); break;
		case 34: sprintf(buffer,"Sabriel"); break;
		case 35: sprintf(buffer,"Lilou"); break;
		case 36: sprintf(buffer,"Chloe"); break;
		case 37: sprintf(buffer,"Zoe"); break;
		case 38: sprintf(buffer,"Louise"); break;
		case 39: sprintf(buffer,"Camille"); break;
		//Funny
		case 40: sprintf(buffer,"Heinrich"); break;
		case 41: sprintf(buffer,"Karl"); break;
		case 42: sprintf(buffer,"Otto"); break;
		case 43: sprintf(buffer,"Siegfried"); break;
		case 44: sprintf(buffer,"Horst"); break;
		case 45: sprintf(buffer,"Hildegard"); break;
		case 46: sprintf(buffer,"Ulrike"); break;
		case 47: sprintf(buffer,"Hendriekje"); break;
		case 48: sprintf(buffer,"Gudrun"); break;
		case 49: sprintf(buffer,"Klarabella"); break;
	}
	return buffer;
}

char* lg_get_combi_name(char* buffer)
{
	char name1[16],name2[16];
	sprintf(buffer,"%s-%s",lg_get_name(name1),lg_get_name(name2));
	return buffer;
}

char lg_new_name[33] = "";

int lg_calc(Uint32 steps)
{
	if (spGetInput()->button[SP_BUTTON_SELECT])
	{
		spGetInput()->button[SP_BUTTON_SELECT] = 0;
		return 1;
	}
	if (spGetInput()->button[SP_BUTTON_UP])
	{
		spGetInput()->button[SP_BUTTON_UP] = 0;
		if (lg_game->player_count >= lg_game->max_player)
			message(lg_font,lg_resize,"Game full!",1,NULL);
		else
		{
			spPollKeyboardInput(lg_new_name,32,SP_BUTTON_UP_MASK);
			if (message(lg_font,lg_resize,"Enter player name:",2,lg_new_name) == 1)
			{
				spStopKeyboardInput();
				if (lg_new_name[0] == 0)
					message(lg_font,lg_resize,"No name entered...",1,NULL);
				else
				if ((lg_last_player->next = join_game(lg_game,lg_new_name,0)) == NULL)
					message(lg_font,lg_resize,"Game full...",1,NULL);
				else
				{
					lg_last_player = lg_last_player->next;
					lg_counter = 10000;
				}
			}
			else
				spStopKeyboardInput();
		}
	}
	if (spGetInput()->button[SP_BUTTON_DOWN])
	{
		spGetInput()->button[SP_BUTTON_DOWN] = 0;
		char leave_name[33];
		sprintf(leave_name,"%s",lg_last_player->name);
		spPollKeyboardInput(leave_name,32,SP_BUTTON_UP_MASK);
		if (message(lg_font,lg_resize,"Enter player name to leave:",2,leave_name) == 1)
		{
			spStopKeyboardInput();
			if (leave_name[0] == 0)
				message(lg_font,lg_resize,"No name entered...",1,NULL);
			else
			{
				//Searching player
				pPlayer l = NULL;
				pPlayer p = lg_player;
				while (p)
				{
					if (strcmp(p->name,leave_name) == 0)
						break;
					l = p;
					p = p->next;
				}
				if (p == NULL)
					message(lg_font,lg_resize,"Player not found",1,NULL);
				else
				{
					pPlayer n = p->next;
					leave_game(p);
					lg_counter = 10000;
					if (l)
						l->next = n;
					else
						lg_player = n;
					if (n == NULL)
						lg_last_player = l;
				}
				if (lg_player == NULL)
					return 1;
			}
		}
		else
			spStopKeyboardInput();
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
			lg_reload();
			hase(lg_resize,lg_game,lg_player);
			return 3;
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
	if (spGetInput()->button[SP_BUTTON_R] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_R] = 0;
		while (lg_ai_list)
		{
			pPlayer next = lg_ai_list->next;
			leave_game(lg_ai_list);
			lg_ai_list = next;
		}
		lg_counter = 10000;
	}
	if (spGetInput()->button[SP_BUTTON_L] && lg_game->admin_pw)
	{
		spGetInput()->button[SP_BUTTON_L] = 0;
		char buffer[32];
		pPlayer ai = join_game(lg_game,lg_get_combi_name(buffer),1);
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
		lg_last_player = lg_player;
		lg_level = NULL;
		lg_level_string[0] = 0;
		lg_block = NULL;
		int res = spLoop(lg_draw,lg_calc,10,resize,NULL);
		if (lg_block)
			spDeleteTextBlock(lg_block);
		if (res == -1)
			message(font,resize,"Game was closed...",1,NULL);
		if (res == 2)
			hase(lg_resize,lg_game,lg_player);
		while (lg_player)
		{
			pPlayer next = lg_player->next;
			leave_game(lg_player);
			lg_player = next;
		}
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

