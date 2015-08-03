#ifndef LOBBYGAME_H
#define LOBBYGAME_H

#include <sparrow3d.h>
#include "lobbyList.h"
#include "client.h"

int game_options(int *game_players,int* game_seconds,int* game_hares,spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ));
void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),pGame game,int spectate);
int lg_reload(void* dummy);

#endif

