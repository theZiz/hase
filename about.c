#include "about.h"
#include "window.h"

void start_about(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"Developer:\nZiz (ziz@mailbox.org)\nGame licensed under GPLv2+\n\n"
	"Fonts:\nPixel Mania by HiBan (CC-BY-SA)\nDejaVu Sans (Public Domain)\n\n"
	"Music:\nAll by Kevin MacLeod (CC-BY-SA)\nwww.incompetech.com\n\n"
	"GFX:\n8 textures by Ziz (CC-BY-SA)\n 1 texture by Binky (CC-BY-SA)\n logo by Comradekingu (CC-BY-SA)\n\n");
	window->height += font->maxheight*16;
	window->only_ok = 1;
	modal_window(window,resize);
	delete_window(window);	
}

const char help_text[] =
	"Hase is a Worms or Artillery like balistic game, but with hares (and other creatures) in space. The gravitation's strength and direction is different at every point in the level depending on the mass nearby and is shown with arrows in the background.\n"
	"Hase is a turn based game. In your turn you have a specific amount of time to deal as much damage to your opponents as possible while collecting items and avoiding mines.\n"
	"\n"
	"You control the hares with [<] and [>]. You can make a long jump with {jump}. If you press {jump} twice you do a high jump.\n"
	"With {weapon} you can choose your weapon. Every use of a weapon needs Action Points - abbreviated AP. At default you start every round with 3 AP. You can use it all at once with one big shot or a lower amount e.g. for aiming tests. In the weapons menu you also get short descriptions of the different weapons.\n"
	"Speaking of it: You aim with [^] and [v]. Before you shoot you should check the power of your weapon shown in the right bottom corner of the screen. You change the power with {power_down} and {power_up}. You finally shoot with {shoot}.\n"
	"\n"
	"If you want to see more of the level while playing press {view} with other buttons. {view} + [<] / [^] / [>] / [v] let's you have a look around. With {view} + {power_down} / {power_up} you can zoom in and out. {view} + {weapon} will speed up the game while {view} + {shoot} will slow it down.\n"
	"If you are unsure what to press while gaming you can always press {view} + {jump} to show an ingame help. With {menu} you open an options menu, from which you can also end the game. If you are playing online - which you should, Hase online doubles the fun! - press {chat} to chat with your opponents.\n"
	"Most of the key bindings can be changed in the options menu - just have a look.\n"
	"\n"
	"Furthermore while creating a game some more game options are available. Usually a hare crossing the level borders dies. However with the infinite mode you are teleported to the other end of the level instead.\n"
	"At default after 15 rounds the Ragnarök begins. This means all items explode and every turn an increasing amount of skulls is spawned, which will slowly destroy the level and kill all remaining players. No nordic gods will save you!\n"
	"To encourage players to make long distant shoots there is also a game mode, in which the strength of the weapons depends on the time of the bullet in air. Shooting a close enemy will do close to no damage, but a long shot can get up to 200% of the original power!\n"
	"Furthermore you can change the default hares per player, health per hare and starting AP.\n"
	"\n"
	"But now enjoy Hase, don't forget to play online from time to time and always remember:\n"
	"   All your hares belong to us!";

spTextBlockPointer help_block;
Sint32 help_scroll;
spFontPointer help_font;

void help_draw(void)
{
	spClearTarget(LL_BG);
	spFontDrawMiddle( spGetWindowSurface()->w/2,   2, 0, "How to Play", help_font );
	int h = spGetWindowSurface()->h - 4 - 4*help_font->maxheight;
	spFontDrawTextBlock(left,2+spGetWindowSurface()->w/6,2*help_font->maxheight+2, 0,help_block,h,help_scroll,help_font);
	if (help_scroll != SP_ONE)
		spFontDraw( 2+spGetWindowSurface()->w*5/6, spGetWindowSurface()->h-2-2*help_font->maxheight, 0, "[v]", help_font );
	if (help_scroll != 0)
		spFontDraw( 2+spGetWindowSurface()->w*5/6,                         2+  help_font->maxheight, 0, "[^]", help_font );
	spFontDrawMiddle( spGetWindowSurface()->w/2,   spGetWindowSurface()->h-2-  help_font->maxheight, 0, "{jump}Okay", help_font );
	spFlip();
}

int help_calc(Uint32 steps)
{
	if (spMapGetByID(MAP_JUMP) || spMapGetByID(MAP_CHAT) || spMapGetByID(MAP_MENU))
	{
		spMapSetByID(MAP_JUMP,0);
		spMapSetByID(MAP_CHAT,0);
		spMapSetByID(MAP_MENU,0);
		return 1;
	}
	if (spGetInput()->axis[1] > 0)
	{
		help_scroll += steps * 32;
		if (help_scroll > SP_ONE)
			help_scroll = SP_ONE;
	}
	if (spGetInput()->axis[1] < 0)
	{
		help_scroll -= steps * 32;
		if (help_scroll < 0)
			help_scroll = 0;
	}
	return 0;
}

void start_help(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	help_block = spCreateTextBlock(help_text,spGetWindowSurface()->w*2/3-4,font);
	help_scroll = 0;
	help_font = font;
	spLoop(help_draw,help_calc,10,resize,NULL);
	
	spDeleteTextBlock(help_block);
}
