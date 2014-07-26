#ifndef MESSAGE_H
#define MESSAGE_H

#include <sparrow3d.h>

int message(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption,int button_count,char* change_char);
int message_cg(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),char* name,int* players,int* seconds,int* online);
void message_draw(void);

#endif
