#include "about.h"
#include "window.h"

void start_about(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	pWindow window = create_window(NULL,font,"Developer:\nZiz (ziz@mailbox.org)\nGame licensed under GPLv2+\n\n"
	"Fonts:\nPixel Mania by HiBan (CC-BY-SA)\nDejaVu Sans (Public Domain)\n\n"
	"Music:\nAll by Kevin MacLeod (CC-BY-SA)\nwww.incompetech.com\n\n"
	"GFX:\n8 textures by Ziz (CC-BY-SA)\n 1 texture by Binky (CC-BY-SA)\n logo by Comradekingu, adapted by Ziz (CC-BY-SA)\n\n");
	window->height += font->maxheight*16;
	window->only_ok = 1;
	modal_window(window,resize);
	delete_window(window);
}

const char help_text[] =
	"Hase is a Worms or Artillery like balistic game, but with hares (and other creatures) in space. The gravitation's strength and direction is different at every point in the level depending on the mass nearby and is shown with arrows in the background.\n"
	"Hase is a turn based game. In your turn you have a specific amount of time to deal as much damage to your opponents as possible while collecting items and avoiding mines.\n"
	"\n"
	"You control the hares with [<] and [>] or the "
#ifdef DESKTOP
	"mouse"
#else
	"touchscreen"
#endif
	". You can make a long jump with {jump}. If you press {jump} twice you do a high jump.\n"
	"With {weapon} or a click on the chosen weapon button you can choose your weapon. Every use of a weapon needs Action Points - abbreviated AP. At default you start every round with 3 AP. You can use it all at once with one big shot or a lower amount e.g. for aiming tests. In the weapons menu you also get short descriptions of the different weapons.\n"
	"Speaking of it: You aim with [^] and [v]"
#ifdef DESKTOP
	" or with the right mouse button"
#endif
	". Before you shoot you should check the power of your weapon shown in the right bottom corner of the screen. You change the power with {power_down} and {power_up}. You finally shoot with {shoot}.\n"
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
	"   All your hare are belong to us!";

spTextBlockPointer help_block;
Sint32 help_scroll;
struct
{
	int x;
	int y;
	int w;
	int h;
} help_button[3]; //exit, up, down
int help_was_pressed;

void help_draw(void)
{
	spClearTarget(LL_BG);
	const int B1 = spMax(spGetSizeFactor()>>16,1);
	const int B2 = spMax(spGetSizeFactor()>>15,1);
	spFontDrawMiddle( spGetWindowSurface()->w/2,   2, 0, "How to Play", help_font );
	int h = spGetWindowSurface()->h - 4 - 4*help_font->maxheight;
	spFontDrawTextBlock(left,2+spGetWindowSurface()->w/6,2*help_font->maxheight+2, 0,help_block,h,help_scroll,help_font);
	if (help_scroll != SP_ONE)
	{
		const char button_text[] = "[v]";
		help_button[2].w = spFontWidth( button_text, help_font );
		help_button[2].h = help_font->maxheight;
		help_button[2].x = 2+spGetWindowSurface()->w*5/6;
		help_button[2].y = spGetWindowSurface()->h-2-2*help_font->maxheight;
		int width = help_button[2].w;
		int height = help_font->maxheight;
		draw_edgy_rectangle(help_button[2].x,help_button[2].y,&width,&height,B1,B2);
		spFontDraw( help_button[2].x, help_button[2].y, 0, button_text, help_font );
	}
	else
	{
		help_button[2].x = -1;
		help_button[2].y = -1;
		help_button[2].w = -1;
		help_button[2].h = -1;
	}
	if (help_scroll != 0)
	{
		const char button_text[] = "[^]";
		help_button[1].w = spFontWidth( button_text, help_font );
		help_button[1].h = help_font->maxheight;
		help_button[1].x = 2+spGetWindowSurface()->w*5/6;
		help_button[1].y = 2+  help_font->maxheight;
		int width = help_button[1].w;
		int height = help_font->maxheight;
		draw_edgy_rectangle(help_button[1].x,help_button[1].y,&width,&height,B1,B2);
		spFontDraw( help_button[1].x, help_button[1].y, 0, button_text, help_font );
	}
	else
	{
		help_button[1].x = -1;
		help_button[1].y = -1;
		help_button[1].w = -1;
		help_button[1].h = -1;
	}
	const char button_text[] = "{jump}Okay";
	help_button[0].w = spFontWidth( button_text, help_font );
	help_button[0].h = help_font->maxheight;
	help_button[0].x = spGetWindowSurface()->w/2 - help_button[0].w/2;
	help_button[0].y = spGetWindowSurface()->h-B2*2-help_font->maxheight;
	int width = help_button[0].w;
	int height = help_font->maxheight;
	draw_edgy_rectangle(help_button[0].x,help_button[0].y,&width,&height,B1,B2);
	spFontDraw( help_button[0].x, help_button[0].y, 0, button_text, help_font );
	spFlip();
}

int help_calc(Uint32 steps)
{
	if ( spGetInput()->touchscreen.pressed )
	{
		help_was_pressed = 1;
		int mx = spGetInput()->touchscreen.x;
		int my = spGetInput()->touchscreen.y;
		int i = 3;
		while (i --> 0 )
		{
			if ( help_button[i].x + help_button[i].w >= mx &&
				help_button[i].x <= mx &&
				help_button[i].y + help_button[i].h >= my &&
				help_button[i].y <= my )
			{
				switch (i)
				{
					case 0:
						spMapSetByID(MAP_JUMP,1);
						break;
					case 1:
						spGetInput()->axis[1] = -1;
						break;
					case 2:
						spGetInput()->axis[1] = +1;
						break;
				}
				break;
			}
		}
	}
	else
	if (help_was_pressed)
	{
		help_was_pressed = 0;
		spGetInput()->axis[1] = 0;
	}
	if (spMapGetByID(MAP_JUMP) || spMapGetByID(MAP_CHAT) || spMapGetByID(MAP_MENU))
	{
		spMapSetByID(MAP_JUMP,0);
		spMapSetByID(MAP_CHAT,0);
		spMapSetByID(MAP_MENU,0);
		spGetInput()->touchscreen.pressed = 0;
		spGetInput()->axis[1] = 0;
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
	int i = 3;
	while (i --> 0 )
	{
		help_button[i].x = -1;
		help_button[i].y = -1;
		help_button[i].w = -1;
		help_button[i].h = -1;
	}
	help_was_pressed = 0;
	help_block = spCreateTextBlock(help_text,spGetWindowSurface()->w*2/3-4,font);
	help_scroll = 0;
	help_font = font;
	spLoop(help_draw,help_calc,10,resize,NULL);

	spDeleteTextBlock(help_block);
}
