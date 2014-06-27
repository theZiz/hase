#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sparrowNet.h>

typedef struct sMessage *pMessage;
typedef struct sMessage
{
	char name[256];
	char content[256];
	int l;
	pMessage next;
} tMessage;

typedef struct sGame *pGame;
typedef struct sGame
{
	int id;
	char name[32];
	char level_string[512];
	int max_player;
	int player_count;
	int create_date;
	int seconds_per_turn;
	int status;
	int admin_pw;
	int mom_player;
	pGame next;
} tGame;

typedef struct sThreadData *pThreadData;

typedef struct sPlayer *pPlayer;
typedef struct sPlayer
{
	int id;
	char name[32];
	int pw;
	int position_in_game;
	pGame game;
	pPlayer next;
	SDL_Thread* input_thread;
	SDL_mutex* input_mutex;
	pThreadData input_data;
	pThreadData last_input_data_write;
	pThreadData last_input_data_read;
	int input_message;
} tPlayer;

typedef struct sThreadData
{
	pPlayer player;
	int second_of_player;
	char data[1536];
	pThreadData next;
} tThreadData;

int server_info();
pGame create_game(char* game_name,int max_player,int seconds_per_turn,char* level_string);
void delete_game_list(pGame game);
void delete_game(pGame game);
int get_games(pGame *gameList);
void delete_player_list(pPlayer player);
pPlayer join_game(pGame game,char* name);
void leave_game(pPlayer player);
void get_game(pGame game,pPlayer *playerList);
void set_status(pGame game,int status);

int push_game(pPlayer player,int second_of_player,void* data);
void push_game_thread(pPlayer player,int second_of_player,void* data);
void start_push_thread();
void end_push_thread();

int pull_game(pPlayer player,int second_of_player,void* data);
int pull_game_thread(pPlayer player,int second_of_player,void* data);
void start_pull_thread(pPlayer player);
void end_pull_thread(pPlayer player);

int connect_to_server();

#endif
