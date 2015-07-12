#ifndef LOBBYLIST_H
#define LOBBYLIST_H

#include <sparrow3d.h>
#include "lobbyGame.h"

#define LL_FG spGetRGB(128,128,160)
#define LL_BG spGetRGB(64,64,80)

void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int start_chat);
int ll_reload(void* dummy);

#endif
