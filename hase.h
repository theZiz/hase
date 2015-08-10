#ifndef HASE_H
#define HASE_H

#include <sparrow3d.h>
#include "client.h"

#define PLAYER_RADIUS 8

int hase(void ( *resize )( Uint16 w, Uint16 h ),pGame game,pPlayer me_list);

#endif
