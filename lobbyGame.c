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
char lg_level_string[1024];
char lg_set_level_string[1024];
char lg_last_level_string[1024];
SDL_Thread* lg_thread = NULL;
int level_mode;

int use_chat;
int after_start;

#define CHAT_LINES 8

void lg_draw(void)
{
	SDL_Surface* screen = spGetWindowSurface();
	spClearTarget(LL_BG);
	char buffer[256];
	int l_w = screen->h-(CHAT_LINES+1)*lg_font->maxheight;
	//Level
	spRectangle  (2+l_w/2, l_w/2, 0,l_w,l_w,LL_FG);
	if (lg_level)
		spBlitSurface(2+l_w/2, l_w/2, 0,lg_level);
	//Heading
	sprintf(buffer, "%s",lg_game->name);
	spFontDrawMiddle(2+l_w/2, 5, 0, buffer, lg_font );
	//Informations
	int w = screen->w-8-l_w;
	sprintf(buffer,"Turn: %is   Hares: %i",lg_game->seconds_per_turn,lg_game->hares_per_player);
	spFontDraw(screen->w-w, 0*lg_font->maxheight, 0, buffer, lg_font );

	if (lg_game->options.bytewise.ragnarok_border & 15)
		sprintf(buffer,"AP: %i   HP: %i   Infinite border",
			((lg_game->options.bytewise.ap_health >> 4)+1),
			((lg_game->options.bytewise.ap_health & 15)+1)*50);
	else
		sprintf(buffer,"AP: %i   HP: %i   Killing border",
			((lg_game->options.bytewise.ap_health >> 4)+1),
			((lg_game->options.bytewise.ap_health & 15)+1)*50);
	spFontDraw(screen->w-w, 1*lg_font->maxheight, 0, buffer, lg_font );
	switch (lg_game->options.bytewise.ragnarok_border >> 4)
	{
		case 0:
			sprintf(buffer,"Ragnarök: Yes");
			break;
		case 7:
			sprintf(buffer,"Ragnarök: No");
			break;
		default:
			sprintf(buffer,"Ragnarök: %i",(lg_game->options.bytewise.ragnarok_border >> 4)*5);
	}
	if (lg_game->options.bytewise.distant_damage)
		sprintf(&buffer[strlen(buffer)],"   Distant dmg: Yes");
	else
		sprintf(&buffer[strlen(buffer)],"   Distant dmg: No");
	spFontDraw(screen->w-w, 2*lg_font->maxheight, 0, buffer, lg_font );
	//player block
	int h = l_w-6*lg_font->maxheight;
	spRectangle(screen->w-4-w/2, 3*lg_font->maxheight+h/2-1, 0,w,h,LL_FG);
	if (lg_block)
		spFontDrawTextBlock(middle,screen->w-w-4, 3*lg_font->maxheight-1, 0,lg_block,h,0,lg_font);
	//Instructions on the right
	//spFontDrawMiddle(screen->w-2-w/2, h+6*lg_font->maxheight, 0, "{weapon}Add player  {view}Remove player", lg_font );
	if (level_mode)
	{
		spFontDraw(screen->w-2-w  , h+3*lg_font->maxheight, 0, "{weapon}New level", lg_font );
		spFontDraw(screen->w-2-w/2, h+3*lg_font->maxheight, 0, "{view}Undo last", lg_font );
		spFontDraw(screen->w-2-w  , h+4*lg_font->maxheight, 0, "{power_down}Load", lg_font );
		spFontDraw(screen->w-2-w/2, h+4*lg_font->maxheight, 0, "{power_up}Save", lg_font );
		spFontDrawMiddle(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "{jump}/{shoot}Set and back", lg_font );
	}
	else
	{
		if (lg_player)
		{
			spFontDraw(screen->w-2-w  , h+3*lg_font->maxheight, 0, "{weapon}Add player", lg_font );
			spFontDraw(screen->w-2-w/2, h+3*lg_font->maxheight, 0, "{view}Remove player", lg_font );
		}
		else
			spFontDrawMiddle(screen->w-2-w/2, h+3*lg_font->maxheight, 0, "Spectate mode!", lg_font );
		
		if (lg_game->admin_pw == 0)
		{
			if (after_start)
				spFontDrawMiddle(screen->w-2-w/2, h+4*lg_font->maxheight, 0, "{jump}View replay", lg_font );
			else
				spFontDrawMiddle(screen->w-2-w/2, h+4*lg_font->maxheight, 0, "Waiting for start...", lg_font );
			spFontDrawMiddle(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "{shoot}Save level", lg_font );
		}
		else
		{
			if (spGetSizeFactor() <= SP_ONE)
			{
				spFontDrawMiddle(screen->w-2-w/2, h+4*lg_font->maxheight, 0, "{power_down}Add AI  {power_up}Game Setup", lg_font );
				spFontDrawMiddle(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "{jump}Start game  {shoot}Level setup", lg_font );
			}
			else
			{
				spFontDraw(screen->w-2-w  , h+4*lg_font->maxheight, 0, "{power_down}Add AI", lg_font );
				spFontDraw(screen->w-2-w/2, h+4*lg_font->maxheight, 0, "{power_up}Game Setup", lg_font );
				spFontDraw(screen->w-2-w  , h+5*lg_font->maxheight, 0, "{jump}Start game", lg_font );
				spFontDraw(screen->w-2-w/2, h+5*lg_font->maxheight, 0, "{shoot}Level setup", lg_font );
			}
		}
	}
	//Chat
	if (lg_game->local)
		spFontDrawMiddle(screen->w/2, l_w+(CHAT_LINES-1)*lg_font->maxheight/2+4, 0,"No chat in local game",lg_font);
	else
	if (use_chat == 0)
		spFontDrawMiddle(screen->w/2, l_w+(CHAT_LINES-1)*lg_font->maxheight/2+4, 0,"Chat deactivated",lg_font);
	else
	if (get_channel() == NULL)
		spFontDrawMiddle(screen->w/2, l_w+(CHAT_LINES-1)*lg_font->maxheight/2+4, 0,"Connecting to IRC...",lg_font);
	else
	{
		spRectangle(screen->w/2, l_w+(CHAT_LINES)*lg_font->maxheight/2+2, 0,screen->w-4,CHAT_LINES*lg_font->maxheight,LL_FG);
		if (lg_chat_block)
			spFontDrawTextBlock(left,4, l_w+2, 0,lg_chat_block,CHAT_LINES*lg_font->maxheight,lg_scroll,NULL);
	}
	//Footline
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
	if (lg_reload_now == 1)
		lg_reload_now = 2;
	if (!lg_game->local && !after_start)
	{
		if (lg_reload_now)
			sprintf(buffer,"Reloading...");
		else
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
	char name1[16];
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

char level_filename[256];

int save_level_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_START_POLL:
			spPollKeyboardInput(level_filename,256,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	sprintf(elem->text,"Filename: %s",level_filename);
	return 0;
}

int overwrite_feedback( pWindow window, pWindowElement elem, int action )
{
	sprintf(elem->text,"Overwrite?");
	return 0;
}

void save_level(char* level_string)
{
	pWindow window = create_window(save_level_feedback,lg_font,"Enter filename for the level");
	add_window_element(window,1,0);
	char complete_path[2048];
	while (1)
	{
		int res = modal_window(window,lg_resize);
		if (res == 1)
		{
			int save = 1;
			if (level_filename[0] == 0)
				message_box(lg_font,lg_resize,"You didn't enter a filename!");
			else
			{
				spCreateDirectoryChain( spConfigGetPath(complete_path,"hase/levels","") );
				if (spFileExists(spConfigGetPath(complete_path,"hase/levels",level_filename)))
				{
					pWindow overwrite_window = create_window(overwrite_feedback,lg_font,"The file does already exist!");
					overwrite_window->cancel_to_no = 1;
					add_window_element(overwrite_window,-1,0);
					if (modal_window(overwrite_window,lg_resize) != 1)
						save = 0;
					delete_window(overwrite_window);
				}
				if (save)
				{
					SDL_RWops *file = SDL_RWFromFile(complete_path, "wb");
					Uint32 magic_number = (Sint32)'E'*0x1000000 + (Sint32)'S'*0x10000 + (Sint32)'A'*0x100 + (Sint32)'H';
					SDL_RWwrite( file, &magic_number, 4 , 1);
					Uint32 l = strlen(level_string);
					SDL_RWwrite( file, &l, 4 , 1);
					SDL_RWwrite( file, level_string, l , 1);
					SDL_RWclose(file);
					char buffer[2048];
					sprintf(buffer,"Saved in %s",spConfigGetPath(complete_path,"hase/levels",""));
					message_box(lg_font,lg_resize,buffer);
					break;
				}
			}
		}
		else
			break;
	}
	delete_window(window);
}

int load_level(char* level_string)
{
	pWindow window = create_window(NULL,lg_font,"Select level to load");
	spFileListPointer flp = NULL;
	char path[2048];
	spFileGetDirectory(&flp,spConfigGetPath(path,"hase/levels",""),0,0);
	int path_l = strlen(path);
	if (flp == NULL)
	{
		char buffer[2048];
		sprintf(buffer,"Nothing found in %s",path);
		message_box(lg_font,lg_resize,buffer);
		return 0;
	}
	spFileSortList(&flp,SP_FILE_SORT_BY_NAME);
	spFileListPointer f = flp;
	int r = 0;
	while (f)
	{
		if (f->type != SP_FILE_DIRECTORY)
		{
			sprintf(add_window_element(window,0,r)->text,"%s",&(f->name[path_l]));
			r++;
		}
		f = f->next;
	}
	if (modal_window(window,lg_resize) == 1)
	{
		f = flp;
		r = 0;
		while (f)
		{
			if (f->type != SP_FILE_DIRECTORY)
			{
				if (r == window->selection)
					break;
				r++;
			}
			f = f->next;
		}
		SDL_RWops *file = SDL_RWFromFile(f->name, "rb");
		Uint32 magic_number;
		SDL_RWread( file, &magic_number, 4 , 1);
		if (magic_number != (Sint32)'E'*0x1000000 + (Sint32)'S'*0x10000 + (Sint32)'A'*0x100 + (Sint32)'H')
		{
			SDL_RWclose(file);
			message_box(lg_font,lg_resize,"No Hase level file!");
			return 0;
		}
		Uint32 l;
		SDL_RWread( file, &l, 4 , 1);
		SDL_RWread( file, level_string, l , 1);
		level_string[l] = 0;
		SDL_RWclose(file);
	}
	spFileDeleteList(flp);
	delete_window(window);
	return 1;
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
		lg_reload_now = 3;
		lg_thread = SDL_CreateThread(lg_reload,NULL);
	}
	if (lg_reload_now == 4)
	{
		int res;
		SDL_WaitThread(lg_thread,&res);
		lg_reload_now = 0;
		lg_counter = 0;
		lg_thread = NULL;
		if (!after_start)
		{
			//Am I still in?
			pPlayer p = lg_player;
			pPlayer l = NULL;
			while (p)
			{
				pPlayer q = lg_player_list;
				while (q)
				{
					if (p->id == q->id)
						break;
					q = q->next;
				}
				if (q == NULL)
				{
					pPlayer n = p->next;
					if (l == NULL && n == NULL) //no prev, no next, I am alone, alone alone, alone alone
						return -4;
					if (!lg_game->local)
						stop_heartbeat(p);
					if (l)
						l->next = n;
					else
						lg_player = n;
					if (n == NULL)
						lg_last_player = l;
					char buffer[256];
					sprintf(buffer,"Player %s was kicked",p->name);
					message_box(lg_font,lg_resize,buffer);
					p = n;
				}
				else
				{
					l = p;
					p = p->next;
				}
			}
			if (res == -1 || res == -2)
				return -1; //stopped
			if (res == -3)
				return -3; //connection error
			if (res == 1)
				return 2; //started
		}
		if (lg_level == NULL || strcmp(lg_level_string,lg_game->level_string))
		{
			spDeleteSurface(lg_level);
			int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
			lg_level = create_level(lg_game->level_string,l_w,l_w,LL_BG);
			sprintf(lg_level_string,"%s",lg_game->level_string);
		}
	}
	if (!lg_game->local && !after_start)
		lg_counter+=steps;
	if (lg_counter >= LG_WAIT && lg_reload_now == 0)
		lg_reload_now = 1;
	if (spMapGetByID(MAP_JUMP) && after_start)
	{
		spMapSetByID(MAP_JUMP,0);
		hase(lg_resize,lg_game,NULL);
		return 3;
	}
	if (spMapGetByID(MAP_SHOOT) && lg_game->admin_pw == 0)
	{
		spMapSetByID(MAP_SHOOT,0);
		save_level(lg_level_string);
	}
	//Spectators are leaving here ;)
	if (lg_player == NULL)
		return 0;
	if (level_mode == 0)
	{
		if (spMapGetByID(MAP_WEAPON))
		{
			spMapSetByID(MAP_WEAPON,0);
			if (text_box(lg_font,lg_resize,"Enter player name:",lg_new_name,32,1,lg_game->sprite_count,0) == 1)
			{
				if (lg_new_name[0] == 0)
					message_box(lg_font,lg_resize,"No name entered...");
				else
				if ((lg_last_player->next = join_game(lg_game,lg_new_name,0,get_last_sprite())) == NULL)
					message_box(lg_font,lg_resize,"Error while joining...");
				else
				{
					lg_last_player = lg_last_player->next;
					if (!lg_game->local)
						start_heartbeat(lg_last_player);
					lg_counter = LG_WAIT;
				}
			}
		}
		if (spMapGetByID(MAP_VIEW))
		{
			spMapSetByID(MAP_VIEW,0);
			pWindow window = create_window(NULL,lg_font,"Which player to remove/kick?");
			pPlayer p;
			if (lg_game->admin_pw)
				p = lg_player_list;
			else
				p = lg_player;
			int r = 0;
			while (p)
			{
				pPlayer q = lg_player;
				while (q)
				{
					if (p->id == q->id)
						break;
					q = q->next;
				}
				if (q)
					sprintf(add_window_element(window,0,r)->text,"%s (local)",p->name);
				else
				if (p->computer)
					sprintf(add_window_element(window,0,r)->text,"%s (AI)",p->name);
				else
					sprintf(add_window_element(window,0,r)->text,"%s (online)",p->name);
				r++;
				p = p->next;
			}
			if (modal_window(window,lg_resize) == 1)
			{
				if (lg_game->admin_pw)
					p = lg_player_list;
				else
					p = lg_player;
				r = 0;
				pPlayer l = NULL;
				pPlayer q = NULL;
				pPlayer n = NULL;
				while (p)
				{
					if (r == window->selection)
					{
						q = lg_player;
						while (q)
						{
							if (p->id == q->id)
								break;
							l = q;
							q = q->next;
						}
						break;
					}
					r++;
					p = p->next;
				}
				if (q) //p is local
				{
					n = q->next;
					if (l == NULL && n == NULL) //no prev, no next, I am alone, alone alone, alone alone
						return 1;
					if (!lg_game->local)
						stop_heartbeat(q);
					if (l)
						l->next = n;
					else
						lg_player = n;
					if (n == NULL)
						lg_last_player = l;
					leave_game(q);
				}
				else
				if (p->computer)
				{
					q = lg_ai_list;
					l = NULL;
					while (q)
					{
						if (p->id == q->id)
							break;
						l = q;
						q = q->next;
					}
					n = q->next;
					if (l)
						l->next = n;
					else
						lg_ai_list = n;
					leave_game(q);
				}
				else
				{
					kick(p);
					SDL_Delay(500); //:P
				}
				lg_counter = LG_WAIT;
			}
			delete_window(window);
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
				int res = lg_reload(NULL);
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
			level_mode = 1;
			memcpy(lg_set_level_string,lg_level_string,1024);
		}
		if (spMapGetByID(MAP_POWER_UP) && lg_game->admin_pw)
		{
			spMapSetByID(MAP_POWER_UP,0);
			Uint32 options = lg_game->options.compressed;
			int seconds_per_turn = lg_game->seconds_per_turn;
			int hares_per_player = lg_game->hares_per_player;
			if (game_options(&options,&seconds_per_turn,&hares_per_player,lg_font,lg_resize) == 1)
				change_game(lg_game,options,seconds_per_turn,hares_per_player);
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
	}
	else //level_mode == 1, admin_pw is set per definition
	{
		if (spMapGetByID(MAP_SHOOT) || spMapGetByID(MAP_JUMP))
		{
			spMapSetByID(MAP_SHOOT,0);
			spMapSetByID(MAP_JUMP,0);
			set_level(lg_game,lg_set_level_string);
			memcpy(lg_level_string,lg_set_level_string,1024);
			level_mode = 0;
		}
		if (spMapGetByID(MAP_WEAPON))
		{
			spMapSetByID(MAP_WEAPON,0);
			memcpy(lg_last_level_string,lg_set_level_string,1024);
			create_level_string(lg_set_level_string,1536,1536,5,5,5);
			spDeleteSurface(lg_level);
			int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
			lg_level = create_level(lg_set_level_string,l_w,l_w,LL_BG);
		}
		if (spMapGetByID(MAP_POWER_UP))
		{
			spMapSetByID(MAP_POWER_UP,0);
			save_level(lg_set_level_string);
		}
		if (spMapGetByID(MAP_POWER_DN))
		{
			spMapSetByID(MAP_POWER_DN,0);
			char buffer[2048];
			memcpy(buffer,lg_set_level_string,1024);
			if (load_level(lg_set_level_string))
			{
				memcpy(lg_last_level_string,buffer,1024);
				spDeleteSurface(lg_level);
				int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
				lg_level = create_level(lg_set_level_string,l_w,l_w,LL_BG);
			}
		}
		if (spMapGetByID(MAP_VIEW))
		{
			spMapSetByID(MAP_VIEW,0);
			char buffer[1024];
			if (lg_last_level_string[0] == 0)
				memcpy(lg_last_level_string,lg_set_level_string,1024);
			else
			{
				memcpy(buffer,lg_last_level_string,1024);
				memcpy(lg_last_level_string,lg_set_level_string,1024);
				memcpy(lg_set_level_string,buffer,1024);
			}
			spDeleteSurface(lg_level);
			int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
			lg_level = create_level(lg_set_level_string,l_w,l_w,LL_BG);
		}
		
			
	}
	return 0;
}

int lg_reload(void* dummy)
{
	if (get_game(lg_game,&lg_player_list)) //Connection error!
	{
		lg_reload_now = 4;
		return -1;
	}
	int l_w = spGetWindowSurface()->h-(4+CHAT_LINES)*lg_font->maxheight;
	char temp[4096] = "";
	pPlayer player = lg_player_list;
	while (player)
	{
		add_to_string(temp,player->name);
		player = player->next;
		if (player)
			add_to_string(temp,", ");
	}
	spTextBlockPointer lg_block_cp = spCreateTextBlock(temp,spGetWindowSurface()->w-8-l_w,lg_font);
	spTextBlockPointer old_lg_block = lg_block;
	lg_block = lg_block_cp;
	if (old_lg_block)
		spDeleteTextBlock(old_lg_block);
	lg_reload_now = 4;
	return lg_game->status;
}

Uint32 lg_game_options = (2 << 4) | 1 | (3 << 12);
int lg_game_seconds = 45;
int lg_game_hares = 3;

int game_options_feedback( pWindow window, pWindowElement elem, int action )
{
	game_options_union options;
	options.compressed = lg_game_options;
	int ap = options.bytewise.ap_health >> 4;
	int health = options.bytewise.ap_health & 15;
	int ragnarok = options.bytewise.ragnarok_border >> 4;
	int border = options.bytewise.ragnarok_border & 15;
	int distant_damage = options.bytewise.distant_damage;
	switch (action)
	{
		case WN_ACT_LEFT:
			switch (elem->reference)
			{
				case 1:
					if (lg_game_seconds > 90)
						lg_game_seconds -= 30;
					else
					if (lg_game_seconds > 5)
						lg_game_seconds -= 5;
					break;
				case 2:
					if (lg_game_hares > 1)
						lg_game_hares--;
					break;
				case 3:
					ap--;
					if (ap < 0)
						ap = 4;
					break;
				case 4:
					health--;
					if (health < 0)
						health = 6;
					break;
				case 5:
					ragnarok--;
					if (ragnarok < 0)
						ragnarok = 7;
					break;
				case 6:
					border = 1-border;
					break;
				case 7:
					distant_damage = 1-distant_damage;
					break;
			}
			break;
		case WN_ACT_RIGHT:
			switch (elem->reference)
			{
				case 1:
					if (lg_game_seconds < 90)
						lg_game_seconds += 5;
					else
						lg_game_seconds += 30;
					break;
				case 2:
					lg_game_hares++;
					break;
				case 3:
					ap++;
					if (ap > 4)
						ap = 0;
					break;
				case 4:
					health++;
					if (health > 6)
						health = 0;
					break;
				case 5:
					ragnarok++;
					if (ragnarok > 7)
						ragnarok = 0;
					break;
				case 6:
					border = 1-border;
					break;
				case 7:
					distant_damage = 1-distant_damage;
					break;
			}
			break;
	}
	switch (elem->reference)
	{
		case 1: sprintf(elem->text,"Seconds per turn: %i",lg_game_seconds); break;
		case 2: sprintf(elem->text,"Hares per player: %i",lg_game_hares); break;
		case 3: sprintf(elem->text,"Start action points: %i",ap+1); break;
		case 4: sprintf(elem->text,"Health: %i",(health+2)*25); break;
		case 5:
			switch (ragnarok)
			{
				case 0: sprintf(elem->text,"Ragnarök: Instant"); break;
				case 7: sprintf(elem->text,"Ragnarök: No"); break;
				default: sprintf(elem->text,"Ragnarök after %i rounds",ragnarok*5);
			}		
			break;
		case 6:
			if (border)
				sprintf(elem->text,"Border behaviour: Infinite");
			else
				sprintf(elem->text,"Border behaviour: Killing");			
			break;
		case 7:
			if (distant_damage)
				sprintf(elem->text,"Long shot extra damage: Yes");
			else
				sprintf(elem->text,"Long shot extra damage: No");			
			break;
	}
	options.bytewise.ap_health = (ap << 4) | (health & 15);
	options.bytewise.ragnarok_border = (ragnarok << 4) | (border & 15);
	options.bytewise.distant_damage = distant_damage;
	lg_game_options = options.compressed;
	return 0;
}

int game_options(Uint32 *game_opt,int* game_seconds,int* game_hares,spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	lg_game_options = *game_opt;
	lg_game_seconds = *game_seconds;
	lg_game_hares = *game_hares;
	pWindow window = create_window(game_options_feedback,font,"Game options");
	add_window_element(window,0,1);
	add_window_element(window,0,2);
	add_window_element(window,0,3);
	add_window_element(window,0,4);
	add_window_element(window,0,5);
	add_window_element(window,0,6);
	add_window_element(window,0,7);
	int result = modal_window(window,resize);
	delete_window(window);	
	*game_opt = lg_game_options;
	*game_seconds = lg_game_seconds;
	*game_hares = lg_game_hares;
	return result;
}

void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), pGame game,int spectate)
{
	level_mode = 0;
	level_filename[0] = 0;
	after_start = (game->status != 0);
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
		if (res == -4)
			message_box(font,resize,"Your last player has been kicked...");
		if (res == 2)
			hase(lg_resize,lg_game,lg_player);

		if (lg_thread)
		{
			SDL_KillThread(lg_thread);
			lg_thread = NULL;
		}

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

