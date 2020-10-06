#ifndef LOBBYLIST_H
#define LOBBYLIST_H

#include <sparrow3d.h>
#include "lobbyGame.h"

#define LL_FG spGetRGB(128,128,160)
#define LL_BG spGetRGB(64,64,80)

extern int use_chat;
extern spTextBlockPointer lg_chat_block;
extern spFontPointer font_dark;
extern spFontPointer font;
extern spFontPointer help_font;
extern SDL_Surface* screen;

void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int start_chat);
int ll_reload(void* dummy);

#endif
