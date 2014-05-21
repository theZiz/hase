/* get_games.php
 * - Gives:
 *   - list of Game names
 *   - list of Game ID
 *   - players of game
 *   - max player
 *   - time per turn
 * - Background:
 *   - deletes abandoned games
 * 
 * create_game.php
 * - Wants:
 *   - game name
 *   - max player
 *   - time per turn
 * - Gives:
 *   - game ID: Random, unique
 *   - admin ID: Random, unique
 * 
 * join_game.php
 * - Wants:
 *   - user name
 *   - game ID
 * - Gives:
 *   - player ID: Random, unique
 *   - time per turn
 * 
 * leave_game.php
 * - Wants:
 *   - player ID
 *   - game ID
 * 
 * push_game.php
 * - Wants:
 *   - player ID
 * 	 - game ID
 *   - turn ID
 *   - second in turn
 *   - last flag
 *   - binary blob of 1000*12 Bit = 1,5kB gamedata
 * - Background:
 *   - increases turn counter if last flag is set
 * 
 * pull_game.php
 * - Wants:
 * 	 - game ID
 *   - turn ID
 *   - second in turn
 * - Gives:
 *   - binary blob of 1000*12 Bit = 1,5kB gamedata
 * 
 * pause_game()
 * - Wants:
 *   - player ID
 *   - game ID
 * 
 * resume_game()
 * - Wants:
 *   - player ID
 *   - game ID
 * 
 * ready.php
 * - Wants:
 *   - game ID
 *   - player ID
 * - Gives
 *   - whether the game is running
 *   - whether it is my turn
 *   - turn ID
 *   - difference between mine and the server time
 *   - whether you are not part of the game (e.g. kicked)
 * - Background:
 *   - Kicks player after timeout
 * 
 * delete_game.php
 * - Wants:
 *   - Game ID
 *   - admin ID
 * 
 * download_replay.php
 * Wants:
 * - game ID
 * Gives:
 * - big binary replay blob*/
#include <stdio.h>
#include <sparrowNet.h>

int main()
{
	spInitNet();
	printf("Init spNet\n");

	//Testing stuff ;)
	spNetIP ip = spNetResolve("ziz.gp2x.de",80);
	printf("IP of ziz.gp2x.de: %i.%i.%i.%i\n",ip.address.ipv4_bytes[0],ip.address.ipv4_bytes[1],ip.address.ipv4_bytes[2],ip.address.ipv4_bytes[3]);
	char buffer[256];
	printf("Host of the IP of ziz.gp2x.de:\"%s\"\n",spNetResolveHost(ip,buffer,256));
	printf("Open Connection to ziz.gp2x.de\n");
	spNetTCPConnection server_connection = spNetOpenClientTCP(ip);
	spNetSendHTTP(server_connection,"POST /haseserver/test.php http/1.1\nHost: ziz.gp2x.de\nUser-Agent: Hase\nContent-Type: application/x-www-form-urlencoded\nContent-Length: 39\n\nname=Alexander+Matthes&email=foo@bar.de\n\n");
	int res = 1;
	while (res)
	{
		res = spNetReceiveHTTP(server_connection,buffer,255);
		buffer[res] = 0;
		printf("%s",buffer);
	}
	printf("Close Connection to ziz.gp2x.de\n");
	spNetCloseTCP(server_connection);
	spQuitNet();
	return 0;
}
