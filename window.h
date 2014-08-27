#ifndef WINDOW_H
#define WINDOW_H

#include <sparrow3d.h>

typedef struct sWindowElement *pWindowElement;
typedef struct sWindowElement
{
	int type;
	int reference;
	char text[256];
	int width;
	pWindowElement next;
} tWindowElement;

typedef struct sWindow *pWindow;
typedef struct sWindow
{
	int width,height;
	int selection;
	int ( *feedback )( pWindowElement elem, int action );
	char title[256];
	spFontPointer font;
	pWindowElement firstElement;
	int do_flip;
	int main_menu;
	int only_ok;
	int count;
} tWindow;

#define WN_ACT_UPDATE 0
#define WN_ACT_LEFT 1
#define WN_ACT_RIGHT 2
#define WN_ACT_START_POLL 3
#define WN_ACT_END_POLL 4


pWindow create_window(int ( *feedback )( pWindowElement elem, int action ),spFontPointer font,char* title);
pWindowElement add_window_element(pWindow window,int type,int reference);
int modal_window(pWindow window, void ( *resize )( Uint16 w, Uint16 h ));
void delete_window(pWindow window);

void message_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption);
int text_box(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ), char* caption, char* text,int len);

int set_message(spFontPointer font, char* caption);
void draw_message_draw(void);

#endif
