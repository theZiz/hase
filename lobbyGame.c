#include "lobbyGame.h"
#include "level.h"
#include <time.h> 
#include "window.h"

#define LG_WAIT 5000

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
spTextBlockPointer lg_chat_block = NULL;
char lg_level_string[512];
pChatMessage lg_last_chat = NULL;
Sint32 lg_scroll;

#define CHAT_LINES 4

void lg_draw(void)
{
	SDL_Surface* screen = spGetWindowSurface();
	spClearTarget(LL_BG);
	char buffer[256];
	//Heading
	if (lg_game->local)
		sprintf(buffer, "%s (Local)",lg_game->name);
	else
		sprintf(buffer, "%s (Internet)",lg_game->name);
	spFontDrawMiddle( screen->w/2, 0*lg_font->maxheight, 0, buffer, lg_font );
	int l_w = screen->h-(4+CHAT_LINES)*lg_font->maxheight;
	//Preview
	spFontDrawMiddle(2+l_w/2, 1*lg_font->maxheight, 0, "Preview", lg_font );
	spRectangle  (2+l_w/2, 2*lg_font->maxheight+l_w/2, 0,l_w,l_w,LL_FG);
	if (lg_level)
		spBlitSurface(2+l_w/2, 2*lg_font->maxheight+l_w/2, 0,lg_level);
	//Informations
	int w = screen->w-8-l_w;
	spFontDrawMiddle(screen->w-2-w/2, 1*lg_font->maxheight, 0, "Game Info", lg_font );
	sprintf(buffer,"Seconds per turn: %i",lg_game->seconds_per_turn);
	spFontDraw(screen->w-w, 2*lg_font->maxheight, 0, buffer, lg_font );
	sprintf(buffer,"Maximum players: %i",lg_game->max_player);
	spFontDraw(screen->w-w, 3*lg_font->maxheight, 0, buffer, lg_font );
	spFontDraw(screen->w-w, 4*lg_font->maxheight, 0, "Players:", lg_font );
	//player block
	int h = l_w-6*lg_font->maxheight;
	spRectangle(screen->w-4-w/2, 5*lg_font->maxheight+h/2-1, 0,w,h,LL_FG);
	if (lg_block)
		spFontDrawTextBlock(middle,screen->w-w-4, 5*lg_font->maxheight-1, 0,lg_block,h,0,lg_font);
	//Instructions on the right
	#ifdef PANDORA
		spFontDrawMiddle(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "[3]Add player  [4]Remove player", lg_font );
	#else
		spFontDrawMiddle(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "[3]Add player  [4]Remove pl.", lg_font );
	#endif
	if (lg_game->admin_pw == 0)
	{
		spFontDrawMiddle(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "The game master will", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, h+7*lg_font->maxheight, 0, "start the game soon™.", lg_font );
	}
	else
	{
		spFontDrawMiddle(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "[l]Add AI  [r]Remove all AIs", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, h+7*lg_font->maxheight, 0, "[o]Start game  [c]New level", lg_font );
	}
	//Chat
	spRectangle(screen->w/2, l_w+(4+CHAT_LINES)*lg_font->maxheight/2+4, 0,screen->w-4,CHAT_LINES*lg_font->maxheight,LL_FG);
	if (lg_game->local)
		spFontDrawMiddle(screen->w/2, l_w+(3+CHAT_LINES)*lg_font->maxheight/2+4, 0,"No chat in local game",lg_font);
	else
	if (lg_chat_block)
		spFontDrawTextBlock(left,4, l_w+2*lg_font->maxheight+4, 0,lg_chat_block,CHAT_LINES*lg_font->maxheight,lg_scroll,lg_font);
	//Footline
	if (lg_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-lg_font->maxheight, 0, "Reloading list...", lg_font );
		lg_reload_now = 2;
	}
	else
	{
		if (lg_game->local)
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[B]Leave and close game", lg_font );
		else
		if (lg_game->admin_pw == 0)
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[R]Chat [B]Leave game", lg_font );
		else
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "[R]Chat [B]Leave and close game", lg_font );
		sprintf(buffer,"Next update: %is",(LG_WAIT-lg_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-lg_font->maxheight, 0, buffer, lg_font );
	}
	spFlip();
}

pPlayer lg_ai_list = NULL;

char* lg_get_name(char* buffer)
{
	int i = spRand()%50;
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
char lg_chat_text[65536];

int lg_calc(Uint32 steps)
{
	if (lg_chat_block)
	{
		if (lg_chat_block->line_count <= CHAT_LINES)
			lg_scroll = 0;
		else
		{
			if (spGetInput()->axis[1] > 0)
			{
				spGetInput()->axis[1] = 0;
				lg_scroll+=SP_ONE/(lg_chat_block->line_count-CHAT_LINES)+1;
				if (lg_scroll > SP_ONE)
					lg_scroll = SP_ONE;
			}
			if (spGetInput()->axis[1] < 0)
			{
				spGetInput()->axis[1] = 0;
				lg_scroll-=SP_ONE/(lg_chat_block->line_count-CHAT_LINES)+1;
				if (lg_scroll < 0)
					lg_scroll = 0;
			}
		}
	}
	if (lg_last_chat != lg_game->chat)
	{
		pChatMessage now = lg_game->chat;
		char temp1[65536] = "";
		char temp2[65536] = "";
		char* t1 = temp1;
		char* t2 = temp2;
		while (now != lg_last_chat)
		{
			char t[32];
			time_t birthtime = now->realtime;
			strftime(t,32,"%T",localtime(&birthtime));
			printf("%s: %s\n",now->name,now->message);
			if (now->next == lg_last_chat)
				sprintf(t1,"%s(%s)%s: %s",t2,t,now->name,now->message);
			else
				sprintf(t1,"%s(%s)%s: %s\n",t2,t,now->name,now->message);
			char* tt = t1;
			t1 = t2;
			t2 = tt;
			now = now->next;
		}
		if (lg_chat_text[0] == 0)
			t1 = t2;
		else
			sprintf(t1,"%s\n%s",lg_chat_text,t2);
		sprintf(lg_chat_text,"%s",t1);
		if (lg_chat_block)
			spDeleteTextBlock(lg_chat_block);
		lg_chat_block = spCreateTextBlock( lg_chat_text, spGetWindowSurface()->w-4, lg_font);
		lg_last_chat = lg_game->chat;
		if (lg_chat_block->line_count > CHAT_LINES)
			lg_scroll = SP_ONE;
	}
	if (!lg_game->local && spGetInput()->button[MY_BUTTON_START])
	{
		spGetInput()->button[MY_BUTTON_START] = 0;
		char m[256] = "";
		if (text_box(lg_font,lg_resize,"Enter Message:",m,256) == 1)
		{
			if (lg_player->next)
			{
				char temp1[256] = "";
				char temp2[256] = "";
				char* t1 = temp1;
				char* t2 = temp2;
				sprintf(t2,"%s",lg_player->name);
				pPlayer p = lg_player->next;
				while (p)
				{
					sprintf(t1,"%s, %s",t2,p->name);
					char* tt = t1;
					t1 = t2;
					t2 = tt;
					p = p->next;	
				}
				send_chat(lg_game,t2,m);
			}
			else
				send_chat(lg_game,lg_player->name,m);
		}
	}
	if (spGetInput()->button[MY_BUTTON_SELECT])
	{
		spGetInput()->button[MY_BUTTON_SELECT] = 0;
		return 1;
	}
	if (spGetInput()->button[MY_PRACTICE_3])
	{
		spGetInput()->button[MY_PRACTICE_3] = 0;
		if (lg_game->player_count >= lg_game->max_player)
			message_box(lg_font,lg_resize,"Game full!");
		else
		{
			if (text_box(lg_font,lg_resize,"Enter player name:",lg_new_name,32) == 1)
			{
				if (lg_new_name[0] == 0)
					message_box(lg_font,lg_resize,"No name entered...");
				else
				if ((lg_last_player->next = join_game(lg_game,lg_new_name,0)) == NULL)
					message_box(lg_font,lg_resize,"Game full...");
				else
				{
					lg_last_player = lg_last_player->next;
					lg_counter = LG_WAIT;
				}
			}
		}
	}
	if (spGetInput()->button[MY_PRACTICE_4])
	{
		spGetInput()->button[MY_PRACTICE_4] = 0;
		char leave_name[33];
		sprintf(leave_name,"%s",lg_last_player->name);
		if (text_box(lg_font,lg_resize,"Enter player name to leave:",leave_name,32) == 1)
		{
			if (leave_name[0] == 0)
				message_box(lg_font,lg_resize,"No name entered...");
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
					message_box(lg_font,lg_resize,"Player not found");
				else
				{
					pPlayer n = p->next;
					leave_game(p);
					lg_counter = LG_WAIT;
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
	}
	if (spGetInput()->button[MY_PRACTICE_OK] && lg_game->admin_pw)
	{
		spGetInput()->button[MY_PRACTICE_OK] = 0;
		if (lg_game->player_count < 2)
			message_box(lg_font,lg_resize,"At least two players are needed!");
		else
		{
			printf("Running game\n");
			set_status(lg_game,1);
			int res = lg_reload();
			if (res == -1)
				return -1; //stopped
			if (res == -3)
				return -3; //connection error
			hase(lg_resize,lg_game,lg_player);
			return 3;
		}
	}
	if (spGetInput()->button[MY_PRACTICE_CANCEL] && lg_game->admin_pw)
	{
		spGetInput()->button[MY_PRACTICE_CANCEL] = 0;
		create_level_string(lg_game->level_string,1536,1536,3,3,3);
		spDeleteSurface(lg_level);
		int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
		lg_level = create_level(lg_game->level_string,l_w,l_w,LL_BG);
		sprintf(lg_level_string,"%s",lg_game->level_string);
		set_level(lg_game,lg_level_string);
	}
	if (spGetInput()->button[MY_BUTTON_R] && lg_game->admin_pw)
	{
		spGetInput()->button[MY_BUTTON_R] = 0;
		while (lg_ai_list)
		{
			pPlayer next = lg_ai_list->next;
			leave_game(lg_ai_list);
			lg_ai_list = next;
		}
		lg_counter = LG_WAIT;
	}
	if (spGetInput()->button[MY_BUTTON_L] && lg_game->admin_pw)
	{
		spGetInput()->button[MY_BUTTON_L] = 0;
		char buffer[32];
		pPlayer ai = join_game(lg_game,lg_get_combi_name(buffer),1);
		if (ai)
		{
			ai->next = lg_ai_list;
			lg_ai_list = ai;
			lg_counter = LG_WAIT;
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
		if (res == -3)
			return -3; //connection error
		if (res == 1)
			return 2; //started
		
	}
	int step;
	for (step = 0; step < steps; step++)
	{
		lg_counter++;
	}
	if (lg_counter >= LG_WAIT)
		lg_reload_now = 1;
	return 0;
}

int lg_reload()
{
	if (get_game(lg_game,&lg_player_list)) //Connection error!
		return -1;
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
	int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
	lg_block = spCreateTextBlock(temp,spGetWindowSurface()->w-8-l_w,lg_font);
	if (lg_level == NULL || strcmp(lg_level_string,lg_game->level_string))
	{
		spDeleteSurface(lg_level);
		lg_level = create_level(lg_game->level_string,l_w,l_w,LL_BG);
		sprintf(lg_level_string,"%s",lg_game->level_string);
	}
	return lg_game->status;
}

void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), pGame game)
{
	lg_game = game;
	lg_font = font;
	lg_counter = LG_WAIT; //instead reload
	lg_local = 0;
	lg_reload_now = 0;
	lg_ai_list = NULL;
	lg_resize = resize;
	if (text_box(font,resize,"Enter player name:",lg_name,32) == 1)
	{
		if (lg_name[0] == 0)
		{
			message_box(font,resize,"No name entered...");
			return;
		}
		if ((lg_player = join_game(game,lg_name,0)) == NULL)
		{
			message_box(font,resize,"Game full...");
			return;
		}
		lg_last_player = lg_player;
		lg_level = NULL;
		lg_level_string[0] = 0;
		lg_block = NULL;
		lg_chat_block = NULL;
		lg_last_chat = NULL;
		lg_chat_text[0] = 0;
		if (!lg_game->local)
			start_chat_listener(lg_player);
		
		int res = spLoop(lg_draw,lg_calc,10,resize,NULL);
		
		if (!lg_game->local)
			stop_chat_listener(lg_player);
		if (lg_block)
			spDeleteTextBlock(lg_block);
		if (res == -1)
			message_box(font,resize,"Game was closed...");
		if (res == -3)
			message_box(font,resize,"Lost connection...");
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
}

