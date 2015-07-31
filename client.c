#include "client.h"
#ifndef WIN32
#include <zlib.h>
#endif
#include "options.h"
#include <stdlib.h>
spNetIP ip;

char hase_url[512] = "";
char hase_gzip = 0;

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

#ifndef WIN32
#define INPUT_COMPRESSION
#endif

int last_heartbeat_diff = -1;

pMessage sendMessage(pMessage message,char* binary_name,void* binary,int count,char* dest,char* server)
{
	char boundary[17] = "A9B8C7D6E5F4G3H2"; //This should maybe random...
	int boundary_len = 16;
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
	length += boundary_len+2+4; //--boundary-- + 2 returns;
	spNetTCPConnection server_connection = spNetOpenClientTCP(ip);
	if (server_connection == NULL)
		return NULL;
	//int buffer_size = 2048+count;//spMax(length+1,512);
	int buffer_size = 65536;
	char in_buffer[buffer_size];
	char* buffer = in_buffer;
	char out_buffer[buffer_size];
	char header[4096]; //magic number ftw!
	char host[512];
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
		#ifdef INPUT_COMPRESSION
		if (hase_gzip)
		{
			//compress data
			z_stream strm;
			strm.zalloc = Z_NULL;
			strm.zfree = Z_NULL;
			strm.opaque = Z_NULL;
			strm.avail_in = count;
			strm.next_in = binary;
			if (deflateInit2 (&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31,8,Z_DEFAULT_STRATEGY) != Z_OK)
			{
				printf("Error: ZStream Init error\n");
				return NULL;
			}
			strm.avail_out = buffer_size;
			strm.next_out = out_buffer;
			switch (deflate(&strm, Z_FINISH))
			{
				case Z_STREAM_ERROR:
					printf("Error: ZStream error\n");
					deflateEnd(&strm);
					return NULL;
				case Z_DATA_ERROR:
					printf("Error: ZStream data error\n");
					return NULL;
				case Z_MEM_ERROR:
					deflateEnd(&strm);
					printf("Error: ZStream mem error\n");
					return NULL;
				case Z_NEED_DICT:
					printf("Error: ZStream need dict error\n");
					return NULL;
			}
			//printf("Compression saved %i%% (%i vs. %i)\n",100-(int)strm.total_out*100/count,(int)strm.total_out,count);
			count = (int)strm.total_out;
			deflateEnd(&strm);
			binary = out_buffer;
		}
		#endif
		length +=
			2+boundary_len+2 + //boundary start + return
			39+strlen(binary_name)+2 + //Content-Disposition: form-data; name="<name>" + return
			count + 4; // content + 2 returns
		int l =
			2+boundary_len+2 + //boundary start + return
			39+strlen(binary_name)+2 + //Content-Disposition: form-data; name="<name>" + return
			4; // content + 2 returns
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
		"\r\n--%s--",boundary);
	memcpy(&(buffer[pos]),temp,2+4+boundary_len+1);
		
	sprintf(host,"%s",server);
	char* server_directory = strchr(host,'/');
	if (server_directory)
	{
		server_directory[0] = 0;
		server_directory++;
		sprintf(header,
			"POST /%s/%s HTTP/1.1\r\n"
			"Origin: http://%s\r\n"
			"User-Agent: Hase\r\n"
			"Connection: Close\r\n"
			"Content-Type: multipart/form-data; boundary=%s\r\n"
			"Content-Length: %i\r\n"
			#ifndef WIN32
			"Accept-Encoding: gzip\r\n"
			#endif
			"Host: %s\r\n"
			"\r\n",server_directory,dest,host,boundary,length,host);
	}
	else
	{
		sprintf(header,
			"POST /%s HTTP/1.1\r\n"
			"Origin: http://%s\r\n"
			"User-Agent: Hase\r\n"
			"Connection: Close\r\n"
			"Content-Type: multipart/form-data; boundary=%s\r\n"
			"Content-Length: %i\r\n"
			#ifndef WIN32
			"Accept-Encoding: gzip\r\n"
			#endif
			"Host: %s\r\n"
			"\r\n",dest,host,boundary,length,host);
	}
	//printf("Header:\n%s",header);
	spNetSendHTTP(server_connection,header);
	//printf("\nMessage\n%sEND\n",buffer);
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
	int encoded = 0;
	int content_length = 0;
	while (
		buffer[pos  ] != '\r' ||
		buffer[pos+1] != '\n' ||
		buffer[pos+2] != '\r' ||
		buffer[pos+3] != '\n')
	{
		if (buffer[pos] == 0)
			return NULL;
		#ifndef WIN32
		if (
			buffer[pos+ 0] == 'C' &&
			buffer[pos+ 1] == 'o' &&
			buffer[pos+ 2] == 'n' &&
			buffer[pos+ 3] == 't' &&
			buffer[pos+ 4] == 'e' &&
			buffer[pos+ 5] == 'n' &&
			buffer[pos+ 6] == 't' &&
			buffer[pos+ 7] == '-')
		{
			if (
				buffer[pos+ 8] == 'E' &&
				buffer[pos+ 9] == 'n' &&
				buffer[pos+10] == 'c' &&
				buffer[pos+11] == 'o' &&
				buffer[pos+12] == 'd' &&
				buffer[pos+13] == 'i' &&
				buffer[pos+14] == 'n' &&
				buffer[pos+15] == 'g' &&
				buffer[pos+16] == ':' &&
				buffer[pos+17] == ' ' &&
				buffer[pos+18] == 'g' &&
				buffer[pos+19] == 'z' &&
				buffer[pos+20] == 'i' &&
				buffer[pos+21] == 'p')
					encoded = 1;
			else
			if (
				buffer[pos+ 8] == 'L' &&
				buffer[pos+ 9] == 'e' &&
				buffer[pos+10] == 'n' &&
				buffer[pos+11] == 'g' &&
				buffer[pos+12] == 't' &&
				buffer[pos+13] == 'h' &&
				buffer[pos+14] == ':' &&
				buffer[pos+15] == ' ')
					content_length = atoi(&buffer[pos+16]);
		}
		#endif
		pos++;
	}
	pos+=4;

	#ifndef WIN32	
	if (encoded)
	{
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = content_length;
		strm.next_in = &in_buffer[pos];
		if (inflateInit2 (&strm, 31) != Z_OK)
		{
			printf("Error: ZStream Init error\n");
			return NULL;
		}
		strm.avail_out = buffer_size-pos;
		strm.next_out = &out_buffer[pos];
		switch (inflate(&strm, Z_NO_FLUSH))
		{
			case Z_STREAM_ERROR:
				printf("Error: ZStream error\n");
				inflateEnd(&strm);
				return NULL;
            case Z_DATA_ERROR:
				printf("Error: ZStream data error\n");
				return NULL;
            case Z_MEM_ERROR:
				inflateEnd(&strm);
				printf("Error: ZStream mem error\n");
				return NULL;
            case Z_NEED_DICT:
				printf("Error: ZStream need dict error\n");
				return NULL;
				
		}
		//printf("Compression saved %i%% (%i vs. %i)\n",100-content_length*100/(int)strm.total_out,content_length,(int)strm.total_out);
		inflateEnd(&strm);
		out_buffer[pos+(int)strm.total_out] = 0;
		buffer = out_buffer;
	}
	else
	#endif
		buffer = in_buffer;
	//printf("%s\n",&buffer[pos]);
	
	if (direction == -1) //incoming file
	{
		if (buffer[pos+1] == 'A' &&
			buffer[pos+2] == 'C' &&
			buffer[pos+3] == 'K')
		{
			memcpy(binary,&buffer[pos+4],count);
			last_heartbeat_diff = *((int*)(&buffer[pos+4+count]));
			pMessage result = NULL;
			numToMessage(&result,"Okay",1);
			return result;
		}
		if (buffer[pos+1] == 'N' &&
			buffer[pos+2] == 'U' &&
			buffer[pos+3] == 'L')
		{
			memset(binary,1+2+16+32,count);
			last_heartbeat_diff = (int)(buffer[pos+4]);
			pMessage result = NULL;
			numToMessage(&result,"Kicked",1);
			return result;
		}
		if (buffer[pos+1] == 'E' &&
			buffer[pos+2] == 'R' &&
			buffer[pos+3] == 'R')
		{
			last_heartbeat_diff = (int)(buffer[pos+4]);
			return NULL;
		}
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
		if (middle && line[0] != '<') //some free hosting website add statistic bullshit
		{
			middle[0] = 0;
			middle++;
			middle++;
			addToMessage(&result,line,middle);
			//printf("Add %s -> %s\n",line,middle);
		}
		if (exit_afterwards)
			break;		
		rest = ++found;
	}
	return result;
}

char irc_server[256] = "";
char irc_channel[256] = "";
int irc_port = 6667;

int server_info()
{
	pMessage result = sendMessage(NULL,NULL,NULL,0,"",gop_server());
	if (result == NULL)
		return -1;
	pMessage mom = result;
	while (mom)
	{
		if (strcmp(mom->name,"gzip") == 0 && strcmp(mom->content,"yes") == 0)
			hase_gzip = 1;
		else
		if (strcmp(mom->name,"irc_server") == 0)
			sprintf(irc_server,"%s",mom->content);
		else
		if (strcmp(mom->name,"irc_channel") == 0)
			sprintf(irc_channel,"%s",mom->content);
		else
		if (strcmp(mom->name,"irc_port") == 0)
			irc_port = atoi(mom->content);
		else
		if (strcmp(mom->name,"version") == 0)
		{
			int version = atoi(mom->content);
			if (version == CLIENT_VERSION)
			{
				sprintf(hase_url,"%s",mom->next->content);
				printf("Using server %s with Version %i\n",hase_url,version);
				deleteMessage(&result);
				return version;
			}
		}
		mom = mom->next;
	}
	deleteMessage(&result);
	return 0;
}

pGame create_game(char* game_name,int max_player,int seconds_per_turn,char* level_string,int local,int hares_per_player)
{
	pMessage result = NULL;
	if (local == 0)
	{
		pMessage message = NULL;
		addToMessage(&message,"game_name",game_name);
		numToMessage(&message,"max_player",max_player);
		numToMessage(&message,"seconds_per_turn",seconds_per_turn);
		numToMessage(&message,"hares_per_player",hares_per_player);
		addToMessage(&message,"level_string",level_string);
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"create_game.php",hase_url);
		deleteMessage(&message);
		if (result == NULL)
			return NULL;
	}
	pGame game = (pGame)malloc(sizeof(tGame));
	game->id = -1;
	sprintf(game->name,"%s",game_name);
	sprintf(game->level_string,"%s",level_string);
	game->max_player = max_player;
	game->player_count = 0;
	game->create_date = 0;
	game->seconds_per_turn = seconds_per_turn;
	game->hares_per_player = hares_per_player;
	game->status = 0;
	game->local = local;
	if (local)
		game->admin_pw = 12345;
	else
		game->admin_pw = 0;
	game->local_player = NULL;
	game->local_counter = 0;
	game->next = NULL;
	memset(game->sprite_count,0,sizeof(int)*SPRITE_COUNT);
	pMessage now = result;
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
	if (game->local == 0)
	{
		pMessage message = NULL;
		numToMessage(&message,"game_id",game->id);
		numToMessage(&message,"admin_pw",game->admin_pw);
		pMessage result = NULL;
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"delete_game.php",hase_url);
		deleteMessage(&message);
		deleteMessage(&result);
	}
	else
		delete_player_list(game->local_player);
	delete_game_list(game);
}

int get_games(pGame *gameList)
{
	pMessage result = NULL;
	int i;
	for (i = 0; i < 3 && result == NULL;i++)
		result = sendMessage(NULL,NULL,NULL,0,"get_games.php",hase_url);
	if (result == NULL)
		return 0;
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
			game->next = NULL;
			game->local = 0;
			memset(game->sprite_count,0,sizeof(int)*SPRITE_COUNT);
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
		if (strcmp(now->name,"hares_per_player") == 0)
			game->hares_per_player = atoi(now->content);
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

pPlayer join_game(pGame game,char* name,int ai,int nr)
{
	if (nr < 1)
		nr = 1;
	if (nr > SPRITE_COUNT)
		nr = SPRITE_COUNT;
	pMessage result = NULL;
	if (game->local == 0)
	{
		pMessage message = NULL;
		addToMessage(&message,"player_name",name);
		numToMessage(&message,"game_id",game->id);
		numToMessage(&message,"computer",ai);
		numToMessage(&message,"nr",nr);
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"join_game.php",hase_url);
		deleteMessage(&message);
		if (result == NULL)
			return NULL;
	}
	pPlayer player = (pPlayer)malloc(sizeof(tPlayer));
	sprintf(player->name,"%s",name);
	player->computer = ai;
	player->id = game->local_counter++;
	player->pw = 0;
	player->game = game;
	player->next = NULL;
	player->input_data = NULL;
	player->last_input_data_write = NULL;
	player->last_input_data_read = NULL;
	player->input_message = 0;
	player->input_mutex = NULL;
	player->input_thread = NULL;
	player->nr = nr;
	player->heartbeat_thread = NULL;
	player->heartbeat_message = 0;
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
	deleteMessage(&result);
	if (game->local)
	{
		int count = 0;
		pPlayer p = game->local_player;
		while (p)
		{
			count++;
			p = p->next;
		}
		if (count >= game->max_player)
		{
			free(player);
			return NULL;
		}
		pPlayer new_player = (pPlayer)malloc(sizeof(tPlayer));
		sprintf(new_player->name,"%s",player->name);
		new_player->computer = player->computer;
		new_player->id = player->id;
		new_player->pw = player->pw;
		new_player->game = player->game;
		new_player->input_data = NULL;
		new_player->last_input_data_write = NULL;
		new_player->last_input_data_read = NULL;
		new_player->input_message = 0;
		new_player->input_mutex = NULL;
		new_player->input_thread = NULL;
		new_player->nr = nr;
		new_player->next = game->local_player;
		new_player->heartbeat_thread = NULL;
		new_player->heartbeat_message = 0;
		game->local_player = new_player;
	}
	game->sprite_count[nr-1]++;
	return player;
}

void leave_game(pPlayer player)
{
	player->game->sprite_count[player->nr-1]--;
	if (player->game->local == 0)
	{
		pMessage message = NULL;
		numToMessage(&message,"game_id",player->game->id);
		numToMessage(&message,"player_id",player->id);
		numToMessage(&message,"player_pw",player->pw);
		pMessage result = NULL;
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"leave_game.php",hase_url);
		deleteMessage(&message);
		deleteMessage(&result);
	}
	else
	{
		pPlayer last_player = NULL;
		pPlayer mom_player = player->game->local_player;
		while (mom_player)
		{
			if (mom_player->id == player->id)
			{
				if (last_player)
					last_player->next = mom_player->next;
				else
					player->game->local_player = mom_player->next;
				free(mom_player);
				break;
			}
			last_player = mom_player;
			mom_player = mom_player->next;
		}
	}
	if (player->input_thread)
		end_pull_thread(player);
	while (player->input_data)
	{
		pThreadData next = player->input_data->next;
		free(player->input_data);
		player->input_data = next;
	}
	free(player);
}

int get_game(pGame game,pPlayer *playerList)
{
	memset(game->sprite_count,0,sizeof(int)*SPRITE_COUNT);
	if (game->local == 0)
	{
		pMessage message = NULL;
		numToMessage(&message,"game_id",game->id);
		pMessage result = NULL;
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"get_game.php",hase_url);
		deleteMessage(&message);
		if (result == NULL)
			return 1;
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
					player->computer = 0;
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
					player->heartbeat_thread = NULL;
					player->heartbeat_message = 0;
				}
				if (strcmp(now->name,"player_name") == 0)
					sprintf(player->name,"%s",now->content);
				if (strcmp(now->name,"computer") == 0)
					player->computer = atoi(now->content);
				if (strcmp(now->name,"position_in_game") == 0)
					player->position_in_game = atoi(now->content);
				if (strcmp(now->name,"nr") == 0)
				{
					player->nr = (atoi(now->content)-1) % SPRITE_COUNT + 1;
					game->sprite_count[player->nr-1]++;
				}
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
			if (strcmp(now->name,"hares_per_player") == 0)
				game->hares_per_player = atoi(now->content);
			if (strcmp(now->name,"status") == 0)
				game->status = atoi(now->content);
			now = now->next;
		}
		deleteMessage(&result);
	}
	else
	{
		if (playerList)
		{
			delete_player_list(*playerList);
			*playerList = NULL;
		}
		pPlayer mom_player = game->local_player;
		game->player_count = 0;
		while (mom_player)
		{
			pPlayer player = (pPlayer)malloc(sizeof(tPlayer));
			player->computer = mom_player->computer;
			player->next = *playerList;
			*playerList = player;
			player->id = mom_player->id;
			player->nr = mom_player->nr;
			game->sprite_count[player->nr-1]++;
			sprintf(player->name,"%s",mom_player->name);
			player->pw = mom_player->pw;
			player->game = mom_player->game;
			player->position_in_game = mom_player->position_in_game;
			player->input_data = NULL;
			player->last_input_data_read = NULL;
			player->last_input_data_write = NULL;
			player->input_message = 0;
			player->input_mutex = NULL;
			player->input_thread = NULL;
			player->heartbeat_thread = NULL;
			player->heartbeat_message = 0;
			mom_player = mom_player->next;
			game->player_count++;
		}
	}
	return 0;
}

void set_status(pGame game,int status)
{
	int old_status = game->status;
	game->status = status;
	if (game->local == 0)
	{
		pMessage message = NULL;
		numToMessage(&message,"game_id",game->id);
		numToMessage(&message,"admin_pw",game->admin_pw);
		numToMessage(&message,"status",game->status);
		pMessage result = NULL;
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"set_status.php",hase_url);
		deleteMessage(&message);
		deleteMessage(&result);	
	}
	else
	if (status == 1 && old_status != 1)
	{
		int positions[game->player_count];
		int i;
		for (i=0;i<game->player_count;i++)
			positions[i] = i;
		//Shuffle
		for (i=0;i<game->player_count*10;i++)
		{
			int a = spRand()%game->player_count;
			int b = spRand()%game->player_count;
			if (a == b)
				continue;
			int temp = positions[a];
			positions[a] = positions[b];
			positions[b] = temp;
		}
		i = 0;
		pPlayer p = game->local_player;
		while (p)
		{
			p->position_in_game = positions[i];
			p = p->next;
			i++;
		}
	}
}

void set_level(pGame game,char* level_string)
{
	sprintf(game->level_string,"%s",level_string);
	if (game->local == 0)
	{
		pMessage message = NULL;
		numToMessage(&message,"game_id",game->id);
		numToMessage(&message,"admin_pw",game->admin_pw);
		addToMessage(&message,"level_string",game->level_string);
		pMessage result = NULL;
		int i;
		for (i = 0; i < 3 && result == NULL;i++)
			result = sendMessage(message,NULL,NULL,0,"set_level.php",hase_url);
		deleteMessage(&message);
		deleteMessage(&result);	
	}
}

int push_game(pPlayer player,int second_of_player,void* data)
{
	if (player->game->local)
		return 0;
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"player_pw",player->pw);
	numToMessage(&message,"second_of_player",second_of_player);
	#ifdef INPUT_COMPRESSION
	if (hase_gzip)
		numToMessage(&message,"gzip",1);
	#endif
	pMessage result = sendMessage(message,"data",data,1536,"push_game.php",hase_url);
	deleteMessage(&message);
	if (result)
	{
		int r = 0;
		if (atoi(result->content) == 1) //kicked!
			r = 2;
		deleteMessage(&result);
		return r;
	}
	return 1; //Error!
}

pThreadData push_thread_first = NULL;
pThreadData push_thread_last = NULL;
SDL_mutex* push_mutex = NULL;
int push_message = 0;

int push_thread_function(void* data)
{
	while (push_message >= 0 || push_thread_first)
	{
		if (push_thread_first)
		{
			pThreadData thread_data = push_thread_first;
			int i = 0;
			if (push_message != -2)
				for (; i < 3; i++)
				{
					int r = push_game(thread_data->player,thread_data->second_of_player,thread_data->data);
					if (r == 0)
						break;
					if (r == 2)
						return 1;
				}
			if (i == 3)
				printf("BIG PANIC at second %i!\n",thread_data->second_of_player);
			else
			{
				if (i != 0)
					printf("Little panic... %i\n",i);
				printf("Sent second %i!\n",thread_data->second_of_player);
				//PULL STACK
				SDL_mutexP(push_mutex);
				push_thread_first = push_thread_first->next;
				if (push_thread_first == NULL)
					push_thread_last = NULL;
				SDL_mutexV(push_mutex);
				free(thread_data);
			}
		}
		else
			spSleep(100000);//100ms
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

int end_push_thread(int kill)
{
	if (kill)
		push_message = -2;
	else
		push_message = -1;
	int result;
	SDL_WaitThread(push_thread,&(result));
	SDL_DestroyMutex(push_mutex);
	push_mutex = NULL;
	push_thread = NULL;
	return result;
}

int pull_game(pPlayer player,int second_of_player,void* data)
{
	if (player->game->local)
		return 0;
	if (player->kicked)
		return 2; //kicked;
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	numToMessage(&message,"second_of_player",second_of_player);
	pMessage result = sendMessage(message,"data",data,-1536,"pull_game.php",hase_url);
	deleteMessage(&message);
	if (result)
	{
		int kicked = 0;
		if (result->name[0] == 'K')
		{
			kicked = 2;
			printf("player %s kicked or not available\n",player->name);
			player->kicked = 1;
		}
		deleteMessage(&result);
		return kicked; //0 Okay, 2 Kicked
	}
	return 1;//Error
}

int pull_thread_function(void* data)
{
	last_heartbeat_diff = -1;
	pPlayer player = data;
	if (player->kicked)
		return 0;
	pThreadData next_data = (pThreadData)malloc(sizeof(tThreadData));
	int pull_count = 0;
	while (player->input_message != -1)
	{
		if (player->kicked)
			break;
		//Try to get second after recent second
		int new_second = 0;
		if (player->last_input_data_write)
			new_second = player->last_input_data_write->second_of_player+1;
		if (pull_game(player,new_second,next_data->data) == 0) //data!
		{
			printf("PULL THREAD: Get second %i of player %s\n",new_second,player->name);
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
			next_data = (pThreadData)malloc(sizeof(tThreadData));
			pull_count++;
			if (pull_count == 4)
			{
				pull_count = 0;
				spSleep(50000); //50ms
			}
		}
		else
			spSleep(500000); //500ms

	}
	free(next_data);
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
	if (thread_data == 0 && player->kicked)
	{
		player->kicked = 2;
		result = 0;
		memset(data,1+2+16+32,1536);
	}
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
	//player->input_data = NULL;
	//player->last_input_data_read = NULL;
	//player->last_input_data_write = NULL;
}

int connect_to_server()
{
	char server[512];
	sprintf(server,"%s",gop_server());
	char* slash;
	if (slash = strchr(server,'/'))
		slash[0] = 0;
	ip = spNetResolve(server,80);
	printf("IP of %s: %i.%i.%i.%i\n",server,ip.address.ipv4_bytes[0],ip.address.ipv4_bytes[1],ip.address.ipv4_bytes[2],ip.address.ipv4_bytes[3]);
	if (ip.address.ipv4 == SP_INVALID_IP)
		return 1;
	return 0;
}

spNetIRCServerPointer server = NULL;
spNetIRCChannelPointer channel = NULL;

void start_irc_client(char* name)
{
	if (server)
		return;
	char buffer[17]; //max nick len on freenode is 16... Let's keep some space for numbers
	int i;
	for (i = 0; i < 15 && name[i];i++)
		if ((((name[i] < '0') || 
			  (name[i] > '9' && name[i] < 'A') ||
			   name[i] > '}') &&
			(name[i] !='-' || i == 0)) ||
			(name[i] >= '0' && name[i] <= '9' && i == 0))
			buffer[i] = '_';
		else
			buffer[i] = name[i];
	buffer[i] = 0;
	printf("%s %i %s\n",buffer,irc_port,irc_server);
	char buffer2[128];
	sprintf(buffer2,SP_DEVICE_STRING);
	for (i = 0; buffer2[i]; i++)
		if (buffer2[i] == ' ')
			buffer2[i] = '_';
	server = spNetIRCConnectServer(irc_server,irc_port,buffer,buffer2,name,"*");
}

int last_query_time = 0;

void try_to_join()
{
	if (channel == NULL && server && spNetIRCServerReady(server))
	{
		channel = spNetIRCJoinChannel(server,irc_channel);
		channel->show_users = 1;
	}
	if (server)
	{
		if (server->first_channel)
		{
			spNetIRCChannelPointer c = server->first_channel;
			while (c)
			{
				if (c != channel && c->close_query == 0)
				{
					int now = time(NULL);
					if (last_query_time < now - 60)
					{
						if (c->last_read_message == NULL)
							c->last_read_message = c->first_message;
						while (c->last_read_message)
						{
							printf("%i Got message from %s: %s\n",now,c->last_read_message->user,c->last_read_message->message);
							c->last_read_message = c->last_read_message->next;
						}
						spNetIRCSendMessage(server,c,"[Automatic reply] This User is within the game \"Hase\" online, which doesn't support queries. The user will not see your message!");
						spNetIRCPartChannel(server,c);
						last_query_time = now;
					}
					break;
				}
				c = c->next;
			}
		}
	}
}

void stop_irc_client()
{
	if (server)
		spNetIRCCloseServer(server);
	server = NULL;
	channel = NULL;
}

spNetIRCChannelPointer get_channel()
{
	if (channel && spNetIRCChannelReady(channel) != 1)
		return NULL;
	return channel;
}

void log_message(char* name,char* message)
{
	char buffer[512];
	FILE * log = fopen(spConfigGetPath(buffer,"hase","chat_log.txt"), "a");
	fprintf(log, "%s: %s\n",name,message);
	fclose(log);
}

void send_chat(pGame game,char* chat_message)
{
	char buffer[512];
	if (game && game->status != -1) //not for replays
		sprintf(buffer,"<%s> %s",game->name,chat_message);
	else
		sprintf(buffer,"%s",chat_message);
	spNetIRCSendMessage(server,channel,buffer);
}

void heartbeat(pPlayer player)
{
	pMessage message = NULL;
	numToMessage(&message,"game_id",player->game->id);
	numToMessage(&message,"player_id",player->id);
	pMessage result = NULL;
	int i;
	for (i = 0; i < 3 && result == NULL;i++)
		result = sendMessage(message,NULL,NULL,0,"heartbeat.php",hase_url);
	if (result == NULL)
		return;
	deleteMessage(&result);
}

int heartbeat_thread_function(void* data)
{
	pPlayer player = data;
	while (player->heartbeat_message != -1)
	{
		heartbeat(player);
		int start = SDL_GetTicks();
		while (SDL_GetTicks() - start < 10000 && player->heartbeat_message != -1)
			spSleep(100000);
	}
	return 0;
}

void start_heartbeat(pPlayer player)
{
	player->heartbeat_message = 0;
	player->heartbeat_thread = SDL_CreateThread(heartbeat_thread_function,(void*)player);	
}

void stop_heartbeat(pPlayer player)
{
	player->heartbeat_message = -1;
	int result;
	SDL_WaitThread(player->heartbeat_thread,&(result));
	player->heartbeat_thread = NULL;
}

char* ingame_message(char* message,char* game_name)
{
	char search[512];
	sprintf(search,"<%s> ",game_name);
	if (strstr(message,search) == message)
	{
		int l;
		for (l = strlen(search);l > 0;l--)
			message++;
		return message;
	}
	return NULL;
}

char music_filename[6][32] =
{
	"./sounds/Ouroboros.ogg",
	"./sounds/Cephalopod.ogg",
	"./sounds/Go Cart.ogg",
	"./sounds/Latin Industries.ogg",
	"./sounds/Pamgaea.ogg",
	"./sounds/Rocket.ogg"
};
int last_music = -1;

void start_random_music()
{
	spSoundStopMusic(0);
	if (last_music == -1)
		last_music = 0;
	else
	{
		int r = last_music;
		while (r == last_music)
			r = rand()%6;
		last_music = r;
	}
	spSoundSetMusic(music_filename[last_music]);
	spSoundPlayMusic(0,-1);
}
