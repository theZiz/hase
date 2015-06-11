#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sparrowNet.h>
#include <sparrow3d.h>
#include <string.h>

#define CLIENT_VERSION 13

#define TRACE_COUNT 16

#define MAP_JUMP 0
#define MAP_SHOOT 1
#define MAP_WEAPON 2
#define MAP_VIEW 3
#define MAP_POWER_UP 4
#define MAP_POWER_DN 5
#define MAP_CHAT 6
#define MAP_MENU 7

#define KEY_POLL_MASK (1 << spMapPoolByID(MAP_JUMP))

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
	Sint32 cam_rotation;
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
	int weapon_points;
	int next_round_extra;
	int kicked;
	SDL_Thread* heartbeat_thread;
	int heartbeat_message;

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
int end_push_thread(int kill);

int pull_game(pPlayer player,int second_of_player,void* data);
int pull_game_thread(pPlayer player,int second_of_player,void* data);
void start_pull_thread(pPlayer player);
void end_pull_thread(pPlayer player);

int connect_to_server();

void log_message(char* name,char* message);

void start_irc_client(char* name);
void stop_irc_client();
void try_to_join();
spNetIRCChannelPointer get_channel();

void send_chat(pGame game,char* chat_message);
void start_heartbeat(pPlayer player);
void stop_heartbeat(pPlayer player);

char* ingame_message(char* message,char* game_name);

#endif
