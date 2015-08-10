#ifndef LOBBYGAME_H
#define LOBBYGAME_H

#include <sparrow3d.h>
#include "lobbyList.h"
#include "client.h"

int game_options(Uint32 *game_opt,int* game_seconds,int* game_hares,spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ));
void save_level(char* level_string);
void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),pGame game,int spectate);
int lg_reload(void* dummy);

#endif

