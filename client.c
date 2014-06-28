#include "client.h"

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
		if (buffer[pos+1] == 'A' &&
			buffer[pos+2] == 'C' &&
			buffer[pos+3] == 'K')
		{
			memcpy(binary,&buffer[pos+4],count);
			pMessage result = NULL;
			numToMessage(&result,"Okay",1);
			return result;
		}
		else
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
		if (player->input_mutex)
		{
			end_pull_thread(player);
			while (player->input_data)
			{
				pThreadData next = player->input_data->next;
				free(player->input_data);
				player->input_data = next;
			}
		}
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
	player->input_data = NULL;
	player->last_input_data_write = NULL;
	player->last_input_data_read = NULL;
	player->input_message = 0;
	player->input_mutex = NULL;
	player->input_thread = NULL;
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
				player->input_data = NULL;
				player->last_input_data_read = NULL;
				player->last_input_data_write = NULL;
				player->input_message = 0;
				player->input_mutex = NULL;
				player->input_thread = NULL;
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

int push_game(pPlayer player,int second_of_player,void* data)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"player_pw",player->pw);
	numToMessage(&message,"second_of_player",second_of_player);
	pMessage result = sendMessage(message,"data",data,1536,"push_game.php");
	deleteMessage(&message);
	if (result)
	{
		deleteMessage(&result);
		return 0;
	}
	return 1; //Error!
}

pThreadData push_thread_first = NULL;
pThreadData push_thread_last = NULL;
SDL_mutex* push_mutex = NULL;
int push_message = 0;

int push_thread_function(void* data)
{
	while (push_message != -1 || push_thread_first)
	{
		if (push_thread_first)
		{
			//PULL STACK
			SDL_mutexP(push_mutex);
			pThreadData thread_data = push_thread_first;
			push_thread_first = push_thread_first->next;
			if (push_thread_first == NULL)
				push_thread_last = NULL;
			SDL_mutexV(push_mutex);
			int i;
			for (i = 0; i < 3; i++)
			{
				if (push_game(thread_data->player,thread_data->second_of_player,thread_data->data) == 0)
					break;
			}
			if (i == 3)
				printf("BIG PANIC!\n");
			else
			if (i != 0)
				printf("Little panic... %i\n",i);
			free(thread_data);
		}
		spSleep(100);
	}
	return 0;
}

void push_game_thread(pPlayer player,int second_of_player,void* data)
{
	pThreadData thread_data = (pThreadData)malloc(sizeof(tThreadData));
	thread_data->player = player;
	thread_data->second_of_player = second_of_player;
	thread_data->next = NULL;
	memcpy(thread_data->data,data,1536);
	//PUSH STACK
	SDL_mutexP(push_mutex);
	if (push_thread_last)
		push_thread_last->next = thread_data;
	push_thread_last = thread_data;
	if (push_thread_first == NULL)
		push_thread_first = thread_data;
	SDL_mutexV(push_mutex);
}

SDL_Thread* push_thread	= NULL;

void start_push_thread()
{
	push_mutex = SDL_CreateMutex();
	push_message = 0;
	push_thread = SDL_CreateThread(push_thread_function,NULL);
}

void end_push_thread()
{
	push_message = -1;
	int result;
	SDL_WaitThread(push_thread,&(result));
	SDL_DestroyMutex(push_mutex);
}

int pull_game(pPlayer player,int second_of_player,void* data)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"second_of_player",second_of_player);
	pMessage result = sendMessage(message,"data",data,-1536,"pull_game.php");
	deleteMessage(&message);
	if (result)
	{
		deleteMessage(&result);
		return 0; //Okay
	}
	return 1;//Error
}

int pull_thread_function(void* data)
{
	pPlayer player = data;
	pThreadData next_data = (pThreadData)malloc(sizeof(tThreadData));
	while (player->input_message != -1)
	{
		//Try to get second after recent second
		int new_second = 0;
		if (player->last_input_data_write)
			new_second = player->last_input_data_write->second_of_player+1;
		if (pull_game(player,new_second,next_data->data) == 0) //data!
		{
			//Adding to the list
			next_data->player = player;
			next_data->second_of_player = new_second;
			next_data->next = NULL;
			SDL_mutexP(player->input_mutex);
			if (player->last_input_data_write)
				player->last_input_data_write->next = next_data;
			else
				player->input_data = next_data;
			player->last_input_data_write = next_data;
			SDL_mutexV(player->input_mutex);
			spSleep(100000); //100ms
		}
		else
			spSleep(1000000); //1s
	}
	return 0;
}

int pull_game_thread(pPlayer player,int second_of_player,void* data)
{
	//Searching
	SDL_mutexP(player->input_mutex);
	pThreadData thread_data = player->last_input_data_read;
	if (player->last_input_data_read == NULL ||
		player->last_input_data_read->second_of_player > second_of_player)
	thread_data = player->input_data;
	while (thread_data)
	{
		if (thread_data->second_of_player == second_of_player)
			break;
		thread_data = thread_data->next;
	}
	int result = 1;
	if (thread_data)
	{
		result = 0;
		memcpy(data,thread_data->data,1536);
	}
	SDL_mutexV(player->input_mutex);
	return result;
}

void start_pull_thread(pPlayer player)
{
	player->input_mutex = SDL_CreateMutex();
	player->input_message = 0;
	player->input_thread = SDL_CreateThread(pull_thread_function,(void*)player);
}

void end_pull_thread(pPlayer player)
{
	player->input_message = -1;
	int result;
	SDL_WaitThread(player->input_thread,&(result));
	SDL_DestroyMutex(player->input_mutex);
	player->input_mutex = NULL;
	player->input_thread = NULL;
	player->input_data = NULL;
	player->last_input_data_read = NULL;
	player->last_input_data_write = NULL;
}

int connect_to_server()
{
	ip = spNetResolve("ziz.gp2x.de",80);
	printf("IP of ziz.gp2x.de: %i.%i.%i.%i\n",ip.address.ipv4_bytes[0],ip.address.ipv4_bytes[1],ip.address.ipv4_bytes[2],ip.address.ipv4_bytes[3]);
	if (ip.address.ipv4 == SP_INVALID_IP)
	{
		spQuitNet();
		return 1;
	}
	return 0;
}

