#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sparrowNet.h>
#include <sparrow3d.h>

#define CLIENT_VERSION 10

#define TRACE_COUNT 16
/*
#define MY_BUTTON_START SP_BUTTON_START
#define MY_BUTTON_SELECT SP_BUTTON_SELECT
#define MY_BUTTON_L SP_BUTTON_L
#define MY_BUTTON_R SP_BUTTON_R
#define MY_PRACTICE_OK SP_PRACTICE_OK
#define MY_PRACTICE_CANCEL SP_PRACTICE_CANCEL
#define MY_PRACTICE_3 SP_PRACTICE_3
#define MY_PRACTICE_4 SP_PRACTICE_4

#define MY_BUTTON_START_NAME SP_BUTTON_START_NAME
#define MY_BUTTON_SELECT_NAME SP_BUTTON_SELECT_NAME
#define MY_BUTTON_L_NAME SP_BUTTON_L_NAME
#define MY_BUTTON_R_NAME SP_BUTTON_R_NAME
#define MY_PRACTICE_OK_NAME SP_PRACTICE_OK_NAME
#define MY_PRACTICE_CANCEL_NAME SP_PRACTICE_CANCEL_NAME
#define MY_PRACTICE_3_NAME SP_PRACTICE_3_NAME
#define MY_PRACTICE_4_NAME SP_PRACTICE_4_NAME

#define KEY_POLL_MASK SP_PRACTICE_OK_MASK*/

#define MY_BUTTON_START SP_BUTTON_START_NOWASD
#define MY_BUTTON_SELECT SP_BUTTON_SELECT_NOWASD
#define MY_BUTTON_L SP_BUTTON_L_NOWASD
#define MY_BUTTON_R SP_BUTTON_R_NOWASD
#define MY_PRACTICE_OK SP_PRACTICE_OK_NOWASD
#define MY_PRACTICE_CANCEL SP_PRACTICE_CANCEL_NOWASD
#define MY_PRACTICE_3 SP_PRACTICE_3_NOWASD
#define MY_PRACTICE_4 SP_PRACTICE_4_NOWASD

#define MY_BUTTON_START_NAME SP_BUTTON_START_NOWASD_NAME
#define MY_BUTTON_SELECT_NAME SP_BUTTON_SELECT_NOWASD_NAME
#define MY_BUTTON_L_NAME SP_BUTTON_L_NOWASD_NAME
#define MY_BUTTON_R_NAME SP_BUTTON_R_NOWASD_NAME
#define MY_PRACTICE_OK_NAME SP_PRACTICE_OK_NOWASD_NAME
#define MY_PRACTICE_CANCEL_NAME SP_PRACTICE_CANCEL_NOWASD_NAME
#define MY_PRACTICE_3_NAME SP_PRACTICE_3_NOWASD_NAME
#define MY_PRACTICE_4_NAME SP_PRACTICE_4_NOWASD_NAME

#define KEY_POLL_MASK SP_PRACTICE_OK_NOWASD_MASK

#define SPRITE_COUNT 18

typedef struct sMessage *pMessage;
typedef struct sMessage
{
	char name[256];
	char content[256];
	int l;
	pMessage next;
} tMessage;

typedef struct sPlayer *pPlayer;

typedef struct sGame *pGame;
typedef struct sGame
{
	int id;
	char name[33];
	char level_string[512];
	int max_player;
	int player_count;
	int create_date;
	int seconds_per_turn;
	int hares_per_player;
	int status;
	int admin_pw;
	pGame next;
	int local;
	pPlayer local_player;
	int local_counter;
	int heartbeat_message;
	SDL_Thread* heartbeat_thread;
	int sprite_count[SPRITE_COUNT];
} tGame;

typedef struct sThreadData *pThreadData;
typedef struct sBullet *pBullet;
typedef struct sBulletTrace *pBulletTrace;

typedef struct sBulletTrace
{
	Sint32 x,y;
	pBullet bullet;
	pBulletTrace next;
} tBulletTrace;

typedef struct sHare *pHare;
typedef struct sHare
{
	int direction;
	int w_direction;
	int w_power;
	Sint32 x,y;
	Sint32 dx,dy;
	Sint32 rotation;
	int bums;
	int hops;
	int high_hops;
	int health;
	spSpriteCollectionPointer hase;	
	pHare before,next;
	int wp_x,wp_y;
} tHare;

typedef struct sPlayer
{
	int id;
	char name[33];
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
	int computer;
	int nr;
	//ingame
	pHare firstHare;
	pHare activeHare;
	pHare setActiveHare;
	int local;
	int time;
	pBulletTrace trace[TRACE_COUNT];
	int tracePos;
	int d_health;
	int d_time;
} tPlayer;

typedef struct sThreadData
{
	pPlayer player;
	int second_of_player;
	char data[1536];
	pThreadData next;
} tThreadData;

int server_info();
pGame create_game(char* game_name,int max_player,int seconds_per_turn,char* level_string,int local,int hares_per_player);
void delete_game_list(pGame game);
void delete_game(pGame game);
int get_games(pGame *gameList);
void delete_player_list(pPlayer player);
pPlayer join_game(pGame game,char* name,int ai,int nr);
void leave_game(pPlayer player);
int get_game(pGame game,pPlayer *playerList);
void set_status(pGame game,int status);
void set_level(pGame game,char* level_string);

int push_game(pPlayer player,int second_of_player,void* data);
void push_game_thread(pPlayer player,int second_of_player,void* data);
void start_push_thread();
void end_push_thread(int kill);

int pull_game(pPlayer player,int second_of_player,void* data);
int pull_game_thread(pPlayer player,int second_of_player,void* data);
void start_pull_thread(pPlayer player);
void end_pull_thread(pPlayer player);

int connect_to_server();


void start_irc_client(char* name);
void stop_irc_client();
void try_to_join();
spNetIRCChannelPointer get_channel();

void send_chat(pGame game,char* chat_message);
void start_heartbeat(pPlayer player);
void stop_heartbeat(pPlayer player);

char* ingame_message(char* message,char* game_name);

#endif
