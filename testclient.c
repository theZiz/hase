#include "client.h"
#include "level.h"

int main(int argc, char **argv)
{
	srand(time(NULL));
	if (argc < 3)
	{
		printf("testclient game nick [players]\n");
		return 0;
	}
	printf("Init spNet\n");
	spInitNet();
	if (connect_to_server())
		return 1;
	printf("Server Version: %i\n",server_info());
	pGame game,gameList = NULL;
	if (argc > 3)
	{
		char buffer[512];
		game = create_game(argv[1],atoi(argv[3]),30,create_level_string(buffer,1536,1536,3,3,3));
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
	player = join_game(game,argv[2]);
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
	}
	else
	while (game->status != 1)
	{
		get_game(game,&playerList);
		spSleep(1000000); //1s
	}
	//Update game last time
	get_game(game,&playerList);
	momplayer = playerList;
	while (momplayer)
	{
		if (momplayer->id == player->id)
			player->position_in_game = momplayer->position_in_game;
		start_pull_thread(momplayer);
		momplayer = momplayer->next;
	}
	printf("I am player %i\n",player->position_in_game);
	
	start_push_thread();
	int i;
	srand(time(0));
	int round;
	for (round = 0; round < 3;round++)
	{
		int p;
		for (p = 0; p < game->player_count; p++)
		{
			int second;
			if (player->position_in_game == p)
			{
				for (second = round*30;second<(round+1)*30;second++)
				{
					char data[1536];
					push_game_thread(player,second,data);
					printf("Player %i sent %i\n",p,second);
					spSleep(rand()%1000000);
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
				for (second = round*30;second<(round+1)*30;second++)
				{
					char data[1536];
					while (pull_game_thread(momplayer,second,data))
						spSleep(100000); //100ms
					printf("Player %i recv %i\n",p,second);
				}				
			}
		}
	}

	
	printf("Waiting for push thread\n");
	end_push_thread();
	printf("Waiting for pull threads\n");
	delete_player_list(playerList);
	
	printf("Finishing...\n");
	leave_game(player);
	if (argc > 3)
		delete_game(game);
	else
		delete_game_list(gameList);
	spQuitNet();
	return 0;
}

