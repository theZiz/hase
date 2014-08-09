#include "about.h"
#include "window.h"

spFontPointer ab_font;
void ( *ab_resize )( Uint16 w, Uint16 h );

void start_about(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"About Hase\n"
	"Version: "VERSION"\n\n"
	"Developer:\nZiz (zizsdl@googlemail.com)\n\n"
	"Fonts:\nPixel Mania by HiBan (CC-BY-SA)\nDejaVu Sans (Public Domain)\n\n"
	"Game licensed under GPLv2+\n\n"
	"All your hares belong to us!");
	window->height += font->maxheight*12;
	modal_window(window,resize);
	delete_window(window);	
}

void start_help(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"Help\n\n"
	"Hase is a game about hares.\n\n"
	"TODO");
	window->height += font->maxheight*4;
	modal_window(window,resize);
	delete_window(window);	
}
