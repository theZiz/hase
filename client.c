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

typedef struct sPlayer *pPlayer;
typedef struct sPlayer
{
	int id;
	char name[32];
	int pw;
	int position_in_game;
	pGame game;
	pPlayer next;
} tPlayer;

spNetIP ip;

void addToMessage(pMessage *message, char* name, char* content)
{
	pMessage temp = (pMessage)malloc(sizeof(tMessage));
	sprintf(temp->name,"%s",name);
	sprintf(temp->content,"%s",content);
	temp->next = *message;
	*message = temp;
}

void numToMessage(pMessage *message, char* name, int number)
{
	char buffer[16];
	sprintf(buffer,"%i",number);
	addToMessage(message,name,buffer);
}

void deleteMessage(pMessage *message)
{
	while (*message)
	{
		pMessage next = (*message)->next;
		free(*message);
		*message = next;
	}
}

pMessage sendMessage(pMessage message,char* binary_name,void* binary,int count,char* dest)
{
	char boundary[17] = "A9B8C7D6E5F4G3H2"; //This should maybe random...
	int boundary_len = 16;
	//Calculating the whole content length
	int direction = 1;
	if (count < 0)
	{
		count *= -1;
		direction = -1;
	}
	int length = 0;
	pMessage mom = message;
	while (mom)
	{
		mom->l =
			2+boundary_len+2 + //boundary start + return
			39+strlen(mom->name)+2 + //Content-Disposition: form-data; name="<name>" + return
			strlen(mom->content)+4; // content + 2 returns
		length += mom->l;
		mom = mom->next;
	}
	if (binary && direction == 1)
	{
/*		length +=
			2+boundary_len+2 + //boundary start + return
			62 + //Content-Disposition: form-data; name=\"userfile\"; filename=\"\"\r\n
			strlen(binary_name) +
			19 + //Content-Type: fun\r\n
			count + 4; // content + 2 returns*/
		length +=
			2+boundary_len+2 + //boundary start + return
			39+strlen(binary_name)+2 + //Content-Disposition: form-data; name="<name>" + return
			count + 4; // content + 2 returns
	}
	length += 2+boundary_len+2+4; //4 + boundary_end + 2 returns;
	spNetTCPConnection server_connection = spNetOpenClientTCP(ip);
	//int buffer_size = 2048+count;//spMax(length+1,512);
	int buffer_size = 65536;
	char buffer[buffer_size];
	sprintf(buffer,
		"POST /haseserver/%s HTTP/1.1\r\n"
		"Origin: http://ziz.gp2x.de\r\n"
		"User-Agent: Hase\r\n"
		"Content-Type: multipart/form-data; boundary=%s\r\n"
		"Content-Length: %i\r\n"
		"Host: ziz.gp2x.de\r\n"
		"\r\n",dest,boundary,length);
	spNetSendHTTP(server_connection,buffer);
	int pos = 0;
	mom = message;
	char temp[512];
	while (mom)
	{
		sprintf(temp,
			"\r\n--%s\r\n"
			"Content-Disposition: form-data; name=\"%s\"\r\n"
			"\r\n"
			"%s",boundary,mom->name,mom->content);
		memcpy(&(buffer[pos]),temp,mom->l);
		pos += mom->l;
		mom = mom->next;
	}
	if (binary && direction == 1)
	{
/*		int l =
			2+boundary_len+2 + //boundary start + return
			62 + //Content-Disposition: form-data; name=\"userfile\"; filename=\"\"\r\n
			strlen(binary_name) +
			19+4; //Content-Type: fun\r\n\r\n		*/
		int l =
			2+boundary_len+2 + //boundary start + return
			39+strlen(binary_name)+2 + //Content-Disposition: form-data; name="<name>" + return
			4; // content + 2 returns
/*		sprintf(temp,
			"\r\n--%s\r\n"
			"Content-Disposition: form-data; name=\"userfile\"; filename=\"%s\"\r\n"
			"Content-Type: fun\r\n\r\n",boundary,binary_name);*/
		sprintf(temp,
			"\r\n--%s\r\n"
			"Content-Disposition: form-data; name=\"%s\"\r\n"
			"\r\n",boundary,binary_name);
		memcpy(&(buffer[pos]),temp,l);
		pos += l;
		memcpy(&(buffer[pos]),binary,count);
		pos += count;
	}
	sprintf(temp,
		"\r\n--%s--\r\n"
		"\r\n",boundary);
	memcpy(&(buffer[pos]),temp,6+4+boundary_len+1);
	spNetSendTCP(server_connection,buffer,length);
	
	int res = spNetReceiveHTTP(server_connection,buffer,buffer_size-1);
	buffer[res] = 0;
	spNetCloseTCP(server_connection);
	//HTTP error check + jumping to begin
	if (
		buffer[ 0] != 'H' ||
		buffer[ 1] != 'T' ||
		buffer[ 2] != 'T' ||
		buffer[ 3] != 'P' ||
		buffer[ 4] != '/' ||
		buffer[ 5] != '1' ||
		buffer[ 6] != '.' ||
		buffer[ 7] != '1' ||
		buffer[ 8] != ' ' ||
		buffer[ 9] != '2' ||
		buffer[10] != '0' ||
		buffer[11] != '0' ||
		buffer[12] != ' ' ||
		buffer[13] != 'O' ||
		buffer[14] != 'K')
		return NULL;
	pos = 15;
	while (
		buffer[pos  ] != '\r' ||
		buffer[pos+1] != '\n' ||
		buffer[pos+2] != '\r' ||
		buffer[pos+3] != '\n')
	{
		if (buffer[pos] == 0)
			return NULL;
		pos++;
	}
	pos+=4;
	
	if (direction == -1) //incoming file TODO
	{
		memcpy(binary,&buffer[pos+1],count);
		return NULL;
	}
	//Parsing in init style
	pMessage result = NULL;
	char* found;
	char* rest = &buffer[pos];
	//printf("_____\n%s\n^^^^^\n",rest);
	while ((found = strchr(rest,'\n')) || (found = strchr(rest,0)))
	{
		char exit_afterwards = 0;
		if (found[0] == 0)
			exit_afterwards = 1;
		found[0] = 0;
		char* line = rest;
		char* middle = strchr(line,':');
		if (middle)
		{
			middle[0] = 0;
			middle++;
			middle++;
			addToMessage(&result,line,middle);
		}
		if (exit_afterwards)
			break;		
		rest = ++found;
	}
	return result;
}

int server_info()
{
	pMessage result = sendMessage(NULL,NULL,NULL,0,"server_info.php");
	if (result == NULL)
		return 0;
	int r = atoi(result->content);
	deleteMessage(&result);
	return r;
}

pGame create_game(char* game_name,int max_player,int seconds_per_turn,char* level_string)
{
	pMessage message = NULL;
	addToMessage(&message,"game_name",game_name);
	numToMessage(&message,"max_player",max_player);
	numToMessage(&message,"seconds_per_turn",seconds_per_turn);
	addToMessage(&message,"level_string",level_string);
	pMessage result = sendMessage(message,NULL,NULL,0,"create_game.php");
	deleteMessage(&message);
	pMessage now = result;
	pGame game = (pGame)malloc(sizeof(tGame));
	game->id = -1;
	sprintf(game->name,"%s",game_name);
	sprintf(game->level_string,"%s",level_string);
	game->max_player = max_player;
	game->player_count = 0;
	game->create_date = 0;
	game->seconds_per_turn = seconds_per_turn;
	game->status = 0;
	game->admin_pw = 0;
	game->mom_player = 0;
	game->next = NULL;
	while (now)
	{
		if (strcmp(now->name,"game_id") == 0)
			game->id = atoi(now->content);
		if (strcmp(now->name,"admin_pw") == 0)
			game->admin_pw = atoi(now->content);
		if (strcmp(now->name,"create_date") == 0)
			game->create_date = atoi(now->content);
		now = now->next;
	}
	deleteMessage(&result);
	return game;
}

void delete_game_list(pGame game)
{
	while (game)
	{
		pGame next = game->next;
		free(game);
		game = next;
	}
}

void delete_game(pGame game)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",game->id);
	numToMessage(&message,"admin_pw",game->admin_pw);
	pMessage result = sendMessage(message,NULL,NULL,0,"delete_game.php");
	deleteMessage(&message);
	deleteMessage(&result);
	delete_game_list(game);
}

int get_games(pGame *gameList)
{
	pMessage result = sendMessage(NULL,NULL,NULL,0,"get_games.php");
	pMessage now = result;
	*gameList = NULL;
	pGame game = NULL;
	int game_count = 0;
	while (now)
	{
		if (strcmp(now->name,"game_count") == 0)
			game_count = atoi(now->content);
		if (strcmp(now->name,"game_id") == 0)
		{
			if (game)
			{
				game->next = (pGame)malloc(sizeof(tGame));
				game = game->next;
			} 
			else
			{
				*gameList = (pGame)malloc(sizeof(tGame));
				game = *gameList;
			}
			game->id = atoi(now->content);
			game->name[0] = 0;
			game->level_string[0] = 0;
			game->max_player = 0;
			game->player_count = 0;
			game->create_date = 0;
			game->seconds_per_turn = 0;
			game->status = 0;
			game->admin_pw = 0;
			game->mom_player = 0;
			game->next = NULL;			
		}
		if (strcmp(now->name,"game_name") == 0)
			sprintf(game->name,"%s",now->content);
		if (strcmp(now->name,"max_player") == 0)
			game->max_player = atoi(now->content);
		if (strcmp(now->name,"player_count") == 0)
			game->player_count = atoi(now->content);
		if (strcmp(now->name,"create_date") == 0)
			game->create_date = atoi(now->content);
		if (strcmp(now->name,"seconds_per_turn") == 0)
			game->seconds_per_turn = atoi(now->content);
		if (strcmp(now->name,"status") == 0)
			game->status = atoi(now->content);
		now = now->next;
	}
	deleteMessage(&result);
	return game_count;
}

void delete_player_list(pPlayer player)
{
	while (player)
	{
		pPlayer next = player->next;
		free(player);
		player = next;
	}
}

pPlayer join_game(pGame game,char* name)
{
	pMessage message = NULL;
	addToMessage(&message,"player_name",name);
	numToMessage(&message,"game_id",game->id);
	pMessage result = sendMessage(message,NULL,NULL,0,"join_game.php");
	pPlayer player = (pPlayer)malloc(sizeof(tPlayer));
	sprintf(player->name,"%s",name);
	player->id = -1;
	player->pw = 0;
	player->game = game;
	player->next = NULL;
	pMessage now = result;
	while (now)
	{
		if (strcmp(now->name,"error") == 0 && atoi(now->content))
		{
			free(player);
			player = NULL;
			break;
		}
		if (strcmp(now->name,"player_id") == 0)
			player->id = atoi(now->content);
		if (strcmp(now->name,"player_pw") == 0)
			player->pw = atoi(now->content);
		now = now->next;
	}
	deleteMessage(&message);
	deleteMessage(&result);
	return player;
}

void leave_game(pPlayer player)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"player_pw",player->pw);
	pMessage result = sendMessage(message,NULL,NULL,0,"leave_game.php");
	deleteMessage(&message);
	deleteMessage(&result);
	delete_player_list(player);
}

void get_game(pGame game,pPlayer *playerList)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",game->id);
	pMessage result = sendMessage(message,NULL,NULL,0,"get_game.php");
	pMessage now = result;
	if (playerList)
	{
		delete_player_list(*playerList);
		*playerList = NULL;
	}
	pPlayer player = NULL;
	while (now)
	{
		if (strcmp(now->name,"player_count") == 0)
			game->player_count = atoi(now->content);
		if (playerList)
		{
			if (strcmp(now->name,"player_id") == 0)
			{
				player = (pPlayer)malloc(sizeof(tPlayer));
				player->next = *playerList;
				*playerList = player;
				player->id = atoi(now->content);
				player->name[0] = 0;
				player->pw = 0;
				player->game = game;
			}
			if (strcmp(now->name,"player_name") == 0)
				sprintf(player->name,"%s",now->content);
			if (strcmp(now->name,"position_in_game") == 0)
				player->position_in_game = atoi(now->content);
		}
		if (strcmp(now->name,"game_name") == 0)
			sprintf(game->name,"%s",now->content);
		if (strcmp(now->name,"level_string") == 0)
			sprintf(game->level_string,"%s",now->content);
		if (strcmp(now->name,"max_player") == 0)
			game->max_player = atoi(now->content);
		if (strcmp(now->name,"player_count") == 0)
			game->player_count = atoi(now->content);
		if (strcmp(now->name,"create_date") == 0)
			game->create_date = atoi(now->content);
		if (strcmp(now->name,"seconds_per_turn") == 0)
			game->seconds_per_turn = atoi(now->content);
		if (strcmp(now->name,"mom_player") == 0)
			game->mom_player = atoi(now->content);
		if (strcmp(now->name,"status") == 0)
			game->status = atoi(now->content);
		now = now->next;
	}
	deleteMessage(&result);
}

void set_status(pGame game,int status)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",game->id);
	numToMessage(&message,"admin_pw",game->admin_pw);
	game->status = status;
	numToMessage(&message,"status",game->status);
	pMessage result = sendMessage(message,NULL,NULL,0,"set_status.php");
	deleteMessage(&message);
	deleteMessage(&result);	
}

void push_game(pPlayer player,int second_of_player,void* data,int last)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"player_pw",player->pw);
	numToMessage(&message,"second_of_player",second_of_player);
	pMessage result = sendMessage(message,"data",data,1536,"push_game.php");
	deleteMessage(&message);
	deleteMessage(&result);
}

void pull_game(pPlayer player,int second_of_player,void* data)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"second_of_player",second_of_player);
	sendMessage(message,"data",data,-1536,"pull_game.php");
	deleteMessage(&message);
}

int main()
{
	printf("Init spNet\n");
	spInitNet();
	ip = spNetResolve("ziz.gp2x.de",80);
	printf("IP of ziz.gp2x.de: %i.%i.%i.%i\n",ip.address.ipv4_bytes[0],ip.address.ipv4_bytes[1],ip.address.ipv4_bytes[2],ip.address.ipv4_bytes[3]);
	if (ip.address.ipv4 == SP_INVALID_IP)
	{
		spQuitNet();
		return 1;
	}
	printf("Server Version: %i\n",server_info());
	pGame game = create_game("Testspiel",2,30,"todo");
	printf("Created game %i with pw %i at time %i\n",game->id,game->admin_pw,game->create_date);
	pPlayer player[3];
	player[0] = join_game(game,"Testuser1");
	player[1] = join_game(game,"Testuser2");
	player[2] = join_game(game,"Testuser3");
	printf("%p %p %p\n",player[0],player[1],player[2]);
	leave_game(player[0]);
	
	pPlayer playerList = NULL;
	get_game(game,&playerList);
	printf("In game %s (status: %i) are %i players\n",game->name,game->status,game->player_count);
	printf("Player:\n");
	pPlayer momplayer = playerList;
	while (momplayer)
	{
		printf("\t%i. %s (%i)\n",momplayer->position_in_game,momplayer->name,momplayer->id);
		momplayer = momplayer->next;
	}
	
	
	player[0] = NULL;
	player[2] = join_game(game,"Testuser3");
	printf("%p %p %p\n",player[0],player[1],player[2]);

	set_status(game,1);
	get_game(game,&playerList);
	printf("In game %s (status: %i) are %i players\n",game->name,game->status,game->player_count);
	printf("Player:\n");
	momplayer = playerList;
	while (momplayer)
	{
		printf("\t%i. %s (%i)\n",momplayer->position_in_game,momplayer->name,momplayer->id);
		momplayer = momplayer->next;
	}
	delete_player_list(playerList);
	char data[1536];
	int i;
	for (i = 0; i < 1536; i++)
		data[i] = i;
	push_game(player[1],0,data,0);
	memset(data,0,1536);
	pull_game(player[1],0,data);
	
	for (i = 0; i < 1536; i++)
		if (data[i] != (char)i)
		{
			printf("Error at %i! Shall be %i, is %i\n",i,(char)i,data[i]);
			break;
		}
	
	delete_game(game);
	
	pGame gameList;
	printf("%i games on the server\n",get_games(&gameList));
	game = gameList;
	while (game)
	{
		get_game(game,NULL);
		printf("%s:\n",game->name);
		printf("\t              ID: %i\n",game->id);
		printf("\t      Max player: %i\n",game->max_player);
		printf("\t      Mom player: %i\n",game->player_count);
		printf("\tSeconds per turn: %i\n",game->seconds_per_turn);
		printf("\t          Status: %i\n",game->status);
		printf("\t     Create time: %i\n",game->create_date);
		printf("\t    Level string: %s\n",game->level_string);
		game = game->next;
	}
	
	spQuitNet();
	return 0;
}
