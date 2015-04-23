#ifndef LOBBYGAME_H
#define LOBBYGAME_H

#include <sparrow3d.h>
#include "lobbyList.h"
#include "client.h"

void start_lobby_game(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),pGame game,int spectate);
int lg_reload();

#endif

