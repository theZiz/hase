#include "about.h"
#include "window.h"

spFontPointer ab_font;
void ( *ab_resize )( Uint16 w, Uint16 h );

void start_about(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"About Hase\n"
	"Version: "VERSION"\n\n"
	"Developer:\nZiz (ziz@mailbox.org)\nGame licensed under GPLv2+\n\n"
	"Fonts:\nPixel Mania by HiBan (CC-BY-SA)\nDejaVu Sans (Public Domain)\n\n"
	"Music:\nOuroboros by Kevin MacLeod (CC-BY-SA)\nwww.incompetech.com");
	window->height += font->maxheight*13;
	window->only_ok = 1;
	modal_window(window,resize);
	delete_window(window);	
}

void start_help(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"Help\n\n"
	"Hase is a game about hares.\n"
	"In Space.\n"
	"With gravity.\n"
	"It is much like Worms. You have a team\n"
	"with different hares and have to kill all\n"
	"hares of all enemy teams. To get an idea\n"
	"about the controls press [R] ingame for help.\n\n"
	"All your hares belong to us!");
	window->height += font->maxheight*10;
	window->only_ok = 1;
	modal_window(window,resize);
	delete_window(window);	
}
