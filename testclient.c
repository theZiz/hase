#include "client.h"
#include "level.h"

#define TURN_LEN 10

int last_shown_time = 0;
pGame game = NULL;

void chat_handling()
{
	if (game->chat)
	{
		pChatMessage chat = game->chat;
		while (chat && chat->birthtime > last_shown_time)
		{
			printf("%i %s: %s\n",last_shown_time,chat->name,chat->message);
			chat = chat->next;
		}
		last_shown_time = game->chat->birthtime;
	}
}

int main(int argc, char **argv)
{
	spSetRand(time(NULL));
	if (argc < 3)
	{
		printf("testclient game nick [players]\n");
		return 0;
	}
	spInitMath();
	printf("Init spNet\n");
	spInitNet();
	if (connect_to_server())
		return 1;
	printf("Server Version: %i\n",server_info());
	pGame gameList = NULL;
	if (argc > 3)
	{
		char buffer[512];
		game = create_game(argv[1],atoi(argv[3]),TURN_LEN,create_level_string(buffer,1536,1536,3,3,3),0);
		printf("Created game %s (%i) with pw %i at time %i\n",game->name,game->id,game->admin_pw,game->create_date);
	}
	else
	{
		//Searching game
		get_games(&gameList);
		game = gameList;
		while (game)
		{
			if (strcmp(game->name,argv[1]) == 0)
				break;
			game = game->next;
		}
		if (game == NULL)
		{
			printf("Can't find game %s\n",argv[1]);
			return 1;
		}
	}
	pPlayer player;
	player = join_game(game,argv[2],0);
	if (player == NULL)
	{
		printf("Game full or already started!\n");
		return 1;
	}
	
	pPlayer playerList = NULL;
	get_game(game,&playerList);
	printf("In game %s (status: %i) are %i players",game->name,game->status,game->player_count);
	pPlayer momplayer = playerList;
	while (momplayer)
	{
		printf(", %s",momplayer->name);
		momplayer = momplayer->next;
	}
	printf("\n");
	//Waiting for game_start
	
	
	if (argc > 3)
	{
		printf("Press return to start...\n");
		getchar();
		set_status(game,1);
		/*int i;
		for (i = 0; i < 6; i++)
		{
			char buffer[512];
			set_level(game,create_level_string(buffer,1536,1536,3,3,3));
			printf("%s\n",buffer);
			sleep(10);
		}*/
	}
	else
	while (game->status != 1)
	{
		get_game(game,&playerList);
		if (game->status == -1)
			goto finish;
		spSleep(1000000); //1s
	}
	
	//Update game last time
	get_game(game,&playerList);
	start_chat_listener(player);
	momplayer = playerList;
	while (momplayer)
	{
		if (momplayer->id == player->id)
			player->position_in_game = momplayer->position_in_game;
		momplayer = momplayer->next;
	}
	printf("I am player %i\n",player->position_in_game);
	
	start_push_thread();
	int i;
	spSetRand(time(0));
	int round;
	for (round = 0; round < 3;round++)
	{
		int p;
		for (p = 0; p < game->player_count; p++)
		{
			while (spRand()%2 == 0)
			{
				send_chat(game,player->name,"Kekskuchen!");
				printf("Send Kekskuchen!\n");
			}
			int second;
			if (player->position_in_game == p)
			{
				for (second = round*TURN_LEN;second<(round+1)*TURN_LEN;second++)
				{
					char data[1536];
					push_game_thread(player,second,data);
					printf("Player %i sent %i\n",p,second);
					spSleep(1000000);
					chat_handling();
				}
			}
			else
			{
				momplayer = playerList;
				while (momplayer)
				{
					if (momplayer->position_in_game == p)
						break;
					momplayer = momplayer->next;
				}
				start_pull_thread(momplayer);
				for (second = round*TURN_LEN;second<(round+1)*TURN_LEN;second++)
				{
					char data[1536];
					while (pull_game_thread(momplayer,second,data))
						spSleep(500000); //500ms
					printf("Player %i recv %i\n",p,second);
					chat_handling();
				}
				end_pull_thread(momplayer);
			}
		}
	}

	
	printf("Waiting for push thread\n");
	end_push_thread();
	delete_player_list(playerList);
	
	printf("Finishing...\n");
	stop_chat_listener(player);
	
	finish:
	leave_game(player);
	if (argc > 3)
		delete_game(game);
	else
		delete_game_list(gameList);
	spQuitNet();
	return 0;
}

