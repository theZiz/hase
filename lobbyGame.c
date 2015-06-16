#include "lobbyGame.h"
#include "level.h"
#include <time.h> 
#include "window.h"

#include "options.h"
#include "client.h"

#include "hase.h"

#include <stdlib.h>

#define LG_WAIT 5000

spFontPointer lg_font;
spFontPointer font_dark;
void ( *lg_resize )( Uint16 w, Uint16 h );
int lg_counter;
pGame lg_game;
int lg_reload_now = 0;
SDL_Surface* lg_level = NULL;
pPlayer lg_player_list = NULL;
pPlayer lg_player;
pPlayer lg_last_player;
spTextBlockPointer lg_block = NULL;
spTextBlockPointer lg_chat_block = NULL;
spNetIRCMessagePointer lg_last_read_message = NULL;
Sint32 lg_scroll;
char lg_level_string[512];

int use_chat;

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
	sprintf(buffer,"Hares per player: %i",lg_game->hares_per_player);
	spFontDraw(screen->w-w, 3*lg_font->maxheight, 0, buffer, lg_font );
	sprintf(buffer,"Maximum players: %i",lg_game->max_player);
	spFontDraw(screen->w-w, 4*lg_font->maxheight, 0, buffer, lg_font );
	sprintf(buffer,"Players: %i",lg_game->player_count);
	spFontDraw(screen->w-w, 5*lg_font->maxheight, 0, buffer, lg_font );
	//player block
	int h = l_w-7*lg_font->maxheight;
	spRectangle(screen->w-4-w/2, 6*lg_font->maxheight+h/2-1, 0,w,h,LL_FG);
	if (lg_block)
		spFontDrawTextBlock(middle,screen->w-w-4, 6*lg_font->maxheight-1, 0,lg_block,h,0,lg_font);
	//Instructions on the right
	//spFontDrawMiddle(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "{weapon}Add player  {view}Remove player", lg_font );
	if (lg_player)
	{
		spFontDraw(screen->w-2-w  , h+6*lg_font->maxheight, 0, "{weapon}Add player", lg_font );
		spFontDraw(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "{view}Remove player", lg_font );
	}
	else
		spFontDrawMiddle(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "Spectate mode!", lg_font );
	
	if (lg_game->admin_pw == 0)
	{
		spFontDrawMiddle(screen->w-2-w/2, h+7*lg_font->maxheight, 0, "The game master will", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, h+8*lg_font->maxheight, 0, "start the game soon™.", lg_font );
	}
	else
	{
		if (spGetSizeFactor() <= SP_ONE)
		{
			spFontDrawMiddle(screen->w-2-w/2, h+7*lg_font->maxheight, 0, "{power_down}Add AI  {power_up}Remove all AIs", lg_font );
			spFontDrawMiddle(screen->w-2-w/2, h+8*lg_font->maxheight, 0, "{jump}Start game  {shoot}New level", lg_font );
		}
		else
		{
			spFontDraw(screen->w-2-w  , h+7*lg_font->maxheight, 0, "{power_down}Add AI", lg_font );
			spFontDraw(screen->w-2-w/2, h+7*lg_font->maxheight, 0, "{power_up}Remove all AIs", lg_font );
			spFontDraw(screen->w-2-w  , h+8*lg_font->maxheight, 0, "{jump}Start game", lg_font );
			spFontDraw(screen->w-2-w/2, h+8*lg_font->maxheight, 0, "{shoot}New level", lg_font );
		}
	}
	//Chat
	if (lg_game->local)
		spFontDrawMiddle(screen->w/2, l_w+(3+CHAT_LINES)*lg_font->maxheight/2+4, 0,"No chat in local game",lg_font);
	else
	if (use_chat == 0)
		spFontDrawMiddle(screen->w/2, l_w+(3+CHAT_LINES)*lg_font->maxheight/2+4, 0,"Chat deactivated",lg_font);
	else
	if (get_channel() == NULL)
		spFontDrawMiddle(screen->w/2, l_w+(3+CHAT_LINES)*lg_font->maxheight/2+4, 0,"Connecting to IRC...",lg_font);
	else
	{
		spRectangle(screen->w/2, l_w+(4+CHAT_LINES)*lg_font->maxheight/2+4, 0,screen->w-4,CHAT_LINES*lg_font->maxheight,LL_FG);
		if (lg_chat_block)
			spFontDrawTextBlock(left,4, l_w+2*lg_font->maxheight+4, 0,lg_chat_block,CHAT_LINES*lg_font->maxheight,lg_scroll,NULL);
	}
	//Footline
	if (lg_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-lg_font->maxheight, 0, "Reloading list...", lg_font );
		lg_reload_now = 2;
	}
	else
	{
		if (lg_game->local || (lg_game->admin_pw && get_channel() == NULL))
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "{menu}Leave and close game", lg_font );
		else
		if (lg_game->admin_pw == 0 && get_channel())
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "{chat}Chat {menu}Leave game", lg_font );
		else
		if (lg_game->admin_pw == 0 && get_channel() == NULL)
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "{menu}Leave game", lg_font );
		else
			spFontDraw( 2, screen->h-lg_font->maxheight, 0, "{chat}Chat {menu}Leave and close game", lg_font );
		if (!lg_game->local)
		{
			sprintf(buffer,"Next update: %is",(LG_WAIT-lg_counter)/1000);
			spFontDrawRight( screen->w-2, screen->h-lg_font->maxheight, 0, buffer, lg_font );
		}
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
	char name1[16];
	//sprintf(buffer,"%s-%s",lg_get_name(name1),lg_get_name(name2));
	sprintf(buffer,"%s",lg_get_name(name1));
	return buffer;
}

char lg_new_name[33] = "";
char lg_chat_text[65536];

int delete_feedback( pWindow window, pWindowElement elem, int action )
{
	sprintf(elem->text,"Do you want to delete the game from the game list?");
	return 0;
}

int lg_calc(Uint32 steps)
{
	try_to_join();
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
	if (get_channel() && get_channel()->first_message)
	{
		char buffer[2048];
		char* message;
		if (lg_last_read_message == NULL)
		{
			if (message = ingame_message(get_channel()->first_message->message,lg_game->name))
			{
				if (strcmp(get_channel()->first_message->ctcp,"ACTION") == 0)
					sprintf(buffer,"*** %s %s",get_channel()->first_message->user,message);
				else
					sprintf(buffer,"%s: %s",get_channel()->first_message->user,message);
				lg_chat_block = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,lg_font);
			}
			else
			if (gop_global_chat())
			{
				if (strcmp(get_channel()->first_message->ctcp,"ACTION") == 0)
					sprintf(buffer,"*** %s %s",get_channel()->first_message->user,get_channel()->first_message->message);
				else
					sprintf(buffer,"%s: %s",get_channel()->first_message->user,get_channel()->first_message->message);
				lg_chat_block = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,font_dark);
			}
			lg_last_read_message = get_channel()->first_message;
		}
		if (lg_last_read_message)
			while (lg_last_read_message->next)
			{
				spNetIRCMessagePointer next = lg_last_read_message->next;
				spTextBlockPointer temp = NULL;
				if (message = ingame_message(next->message,lg_game->name))
				{
					if (strcmp(next->ctcp,"ACTION") == 0)
						sprintf(buffer,"*** %s %s",next->user,message);
					else
						sprintf(buffer,"%s: %s",next->user,message);
					temp = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,lg_font);
				}
				else
				if (gop_global_chat())
				{
					if (strcmp(next->ctcp,"ACTION") == 0)
						sprintf(buffer,"*** %s %s",next->user,next->message);
					else
						sprintf(buffer,"%s: %s",next->user,next->message);
					temp = spCreateTextBlock(buffer,spGetWindowSurface()->w-4,font_dark);
				}
				if (temp)
				{
					if (lg_chat_block)
					{
						int lc = lg_chat_block->line_count + temp->line_count;
						spTextLinePointer copyLine = (spTextLinePointer)malloc(lc*sizeof(spTextLine));
						memcpy(copyLine,lg_chat_block->line,lg_chat_block->line_count*sizeof(spTextLine));
						memcpy(&copyLine[lg_chat_block->line_count],temp->line,temp->line_count*sizeof(spTextLine));
						free(lg_chat_block->line);
						lg_chat_block->line = copyLine;
						lg_chat_block->line_count = lc;
						free(temp);
					}
					else
						lg_chat_block = temp;
					if (lg_chat_block->line_count > CHAT_LINES)
						lg_scroll = SP_ONE;
				}
				lg_last_read_message = next;
			}
	}
	if (!lg_game->local && get_channel() && spMapGetByID(MAP_CHAT))
	{
		spMapSetByID(MAP_CHAT,0);
		char m[256] = "";
		if (text_box(lg_font,lg_resize,"Enter Message:",m,256,0,NULL,1) == 1)
			send_chat(lg_game,m);
	}
	if (spMapGetByID(MAP_MENU))
	{
		spMapSetByID(MAP_MENU,0);
		return 1;
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
	if (!lg_game->local)
		lg_counter+=steps;
	if (lg_counter >= LG_WAIT)
		lg_reload_now = 1;
	//Spectators are leaving here ;)
	if (lg_player == NULL)
		return 0;
	if (spMapGetByID(MAP_WEAPON))
	{
		spMapSetByID(MAP_WEAPON,0);
		if (lg_game->player_count >= lg_game->max_player)
			message_box(lg_font,lg_resize,"Game full!");
		else
		{
			if (text_box(lg_font,lg_resize,"Enter player name:",lg_new_name,32,1,lg_game->sprite_count,0) == 1)
			{
				if (lg_new_name[0] == 0)
					message_box(lg_font,lg_resize,"No name entered...");
				else
				if ((lg_last_player->next = join_game(lg_game,lg_new_name,0,get_last_sprite())) == NULL)
					message_box(lg_font,lg_resize,"Game full...");
				else
				{
					lg_last_player = lg_last_player->next;
					if (!lg_game->local)
						start_heartbeat(lg_last_player);
					lg_counter = LG_WAIT;
				}
			}
		}
	}
	if (spMapGetByID(MAP_VIEW))
	{
		spMapSetByID(MAP_VIEW,0);
		char leave_name[33];
		sprintf(leave_name,"%s",lg_last_player->name);
		if (text_box(lg_font,lg_resize,"Enter player name to leave:",leave_name,32,0,NULL,0) == 1)
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
					if (l == NULL && n == NULL) //no prev, no next
						return 1;
					if (!lg_game->local)
						stop_heartbeat(p);
					leave_game(p);
					lg_counter = LG_WAIT;
					if (l)
						l->next = n;
					else
						lg_player = n;
					if (n == NULL)
						lg_last_player = l;
				}
			}
		}
	}
	if (spMapGetByID(MAP_JUMP) && lg_game->admin_pw)
	{
		spMapSetByID(MAP_JUMP,0);
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
			if (hase(lg_resize,lg_game,lg_player) < 2 && lg_game->local == 0)
			{
				pWindow window = create_window(delete_feedback,lg_font,"Question");
				window->cancel_to_no = 1;
				add_window_element(window,-1,0);
				int res = modal_window(window,lg_resize);
				delete_window(window);
				if (res == 1)
					set_status(lg_game,-2);
			}
			return 3;
		}
	}
	if (spMapGetByID(MAP_SHOOT) && lg_game->admin_pw)
	{
		spMapSetByID(MAP_SHOOT,0);
		create_level_string(lg_game->level_string,1536,1536,3,3,3);
		spDeleteSurface(lg_level);
		int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
		lg_level = create_level(lg_game->level_string,l_w,l_w,LL_BG);
		sprintf(lg_level_string,"%s",lg_game->level_string);
		set_level(lg_game,lg_level_string);
	}
	if (spMapGetByID(MAP_POWER_UP) && lg_game->admin_pw)
	{
		spMapSetByID(MAP_POWER_UP,0);
		while (lg_ai_list)
		{
			pPlayer next = lg_ai_list->next;
			leave_game(lg_ai_list);
			lg_ai_list = next;
		}
		lg_counter = LG_WAIT;
	}
	if (spMapGetByID(MAP_POWER_DN) && lg_game->admin_pw)
	{
		spMapSetByID(MAP_POWER_DN,0);
		char buffer[32];
		pPlayer ai = join_game(lg_game,lg_get_combi_name(buffer),1,rand()%SPRITE_COUNT+1);
		if (ai)
		{
			ai->next = lg_ai_list;
			lg_ai_list = ai;
			lg_counter = LG_WAIT;
		}
	}
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

void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), pGame game,int spectate)
{
	lg_game = game;
	lg_font = font;
	lg_counter = LG_WAIT; //instead reload
	lg_reload_now = 0;
	lg_ai_list = NULL;
	lg_last_read_message = get_channel()?get_channel()->last_read_message:NULL;
	lg_resize = resize;
	if (spectate ||
		game->local == 1 && text_box(font,resize,"Enter player name:",gop_username(),32,1,NULL,0) == 1 ||
		game->local == 0 && sprite_box(font,resize,"Choose sprite!",1,game->admin_pw?NULL:game->sprite_count) == 1)
	{
		if (!spectate)
		{
			if (gop_username()[0] == 0)
			{
				message_box(font,resize,"No name entered...");
				return;
			}
			save_options();
			if ((lg_player = join_game(game,gop_username(),0,get_last_sprite())) == NULL)
			{
				message_box(font,resize,"Game full...");
				return;
			}
			lg_last_player = lg_player;
		}
		else
			lg_player = NULL;
		lg_level = NULL;
		lg_level_string[0] = 0;
		lg_block = NULL;
		lg_chat_block = NULL;
		lg_chat_text[0] = 0;
		if (!spectate && !lg_game->local)
			start_heartbeat(lg_player);
		
		int res = spLoop(lg_draw,lg_calc,10,resize,NULL);
		
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
			if (!spectate && !lg_game->local)
				stop_heartbeat(lg_player);
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

