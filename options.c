#include "options.h"
#include "window.h"
#include "mapping.h"
#include <stdlib.h>
#include <string.h>
#include "client.h"

int op_circle = 1;
int op_music_volume = SP_VOLUME_MAX << (VOLUME_SHIFT-1);
int op_sample_volume = SP_VOLUME_MAX << VOLUME_SHIFT;
int op_particles = 1;
int op_rotation = 1;
int op_direction_flip = 0;
int op_show_names = 1;
int op_show_map = 1;
int op_global_chat = 1;
char op_server[512] = "hase.schmidt-matthes.de/hase.php";
char op_username[32] = SP_DEVICE_STRING" User";
Uint32 op_game_options = (2 << 0) | (2 << 4) | (0 << 8) | (3 << 12) | (0 << 16) | (8 << 20) | (7 << 24);
int op_game_seconds = 45;
int op_game_hares = 3;
int op_first_game = 1;
int op_sprite = 0;
int op_update_server = 0;

void sop_update_server(int v)
{
	op_update_server = v;
}

int gop_update_server()
{
	return op_update_server;
}

int gop_first_game()
{
	return op_first_game;
}

void sop_first_game(int g)
{
	op_first_game = g;
}

int gop_show_names()
{
	return op_show_names;
}

int gop_show_map()
{
	return op_show_map;
}

int gop_global_chat()
{
	return op_global_chat;
}

void sop_show_names(int v)
{
	op_show_names = v;
}

void sop_show_map(int v)
{
	op_show_map = v;
}

void sop_global_chat(int v)
{
	op_global_chat = v;
}

int gop_circle()
{
	return op_circle;
}

int gop_music_volume()
{
	return op_music_volume;
}

int gop_sample_volume()
{
	return op_sample_volume;
}

int gop_particles()
{
	return op_particles;
}

int gop_rotation()
{
	return op_rotation;
}

int gop_direction_flip()
{
	return op_direction_flip;
}

char* gop_server()
{
	return op_server;
}

char* gop_username()
{
	return op_username;
}

Uint32* gop_game_options_ptr()
{
	return &op_game_options;
}

int* gop_game_hares_ptr()
{
	return &op_game_hares;
}

int* gop_game_seconds_ptr()
{
	return &op_game_seconds;
}

int gop_sprite()
{
	return op_sprite;
}

void sop_circle(int v)
{
	op_circle = v & 1;
}

void sop_music_volume(int v)
{
	if (v < 0)
		v = 0;
	if (v > (SP_VOLUME_MAX << VOLUME_SHIFT))
		v = SP_VOLUME_MAX << VOLUME_SHIFT;
	op_music_volume = v;
	spSoundSetMusicVolume(v >> (VOLUME_SHIFT+3));
}

void sop_sample_volume(int v)
{
	if (v < 0)
		v = 0;
	if (v > (SP_VOLUME_MAX << VOLUME_SHIFT))
		v = SP_VOLUME_MAX << VOLUME_SHIFT;
	op_sample_volume = v;
	spSoundSetVolume(v >> VOLUME_SHIFT);
}

void sop_particles(int v)
{
	if (v < 1)
		v = 1;
	if (v > 4)
		v = 4;
	op_particles = v;
}

void sop_rotation(int v)
{
	op_rotation = v & 1;
}

void sop_direction_flip(int v)
{
	op_direction_flip = v & 1;
}

void sop_server(char* server)
{
	sprintf(op_server,"%s",server);
}

void sop_username(char* username)
{
	snprintf(op_username,32,"%s",username);
}

void sop_game_options(Uint32 options)
{
	game_options_union opu;
	opu.compressed = options;
	int health = opu.bytewise.ap_health & 15;
	int ap = opu.bytewise.ap_health >> 4;
	int border = opu.bytewise.ragnarok_border & 15;
	int ragnarok = opu.bytewise.ragnarok_border >> 4;
	int distant_damage = opu.bytewise.distant_damage_handicap_count & 15;
	int hdc_count = opu.bytewise.distant_damage_handicap_count >> 4;
	int hdc_health = opu.bytewise.handicap_health & 15;

	if (ap > 4)
		ap = 4;
	if (health > 6)
		health = 6;
	if (ragnarok > 7)
		ragnarok = 7;
	if (border > 1)
		border = 1;
	if (distant_damage > 1)
		distant_damage = 1;
	if (hdc_health == 0)
		hdc_health = 7;
	if (hdc_count == 0)
		hdc_count = 8;
	opu.bytewise.ap_health = (ap << 4) | (health & 15);
	opu.bytewise.ragnarok_border = (ragnarok << 4) | (border & 15);
	opu.bytewise.distant_damage_handicap_count = (hdc_count << 4) | (distant_damage & 15);
	opu.bytewise.handicap_health = (hdc_health & 15);
	op_game_options = opu.compressed;
}

void sop_game_hares(int hares)
{
	op_game_hares = spMax(1,hares);
}

void sop_game_seconds(int seconds)
{
	op_game_seconds = spMax(1,seconds/5)*5;
}

void sop_sprite(int v)
{
	op_sprite = spMax(0,spMin(v,SPRITE_COUNT-1));
}

void load_options()
{
	spConfigPointer conf = spConfigRead("config.ini","hase");
	spConfigEntryPointer entry = conf->firstEntry;
	int found_username = 0;
	while (entry)
	{
		if (strcmp(entry->key,"username") == 0)
		{
			found_username = 1;
			sop_username(entry->value);
		}
		if (strcmp(entry->key,"server") == 0)
			sop_server(entry->value);
		if (strcmp(entry->key,"circle") == 0)
			sop_circle(atoi(entry->value));
		if (strcmp(entry->key,"music_volume") == 0)
			sop_music_volume(atoi(entry->value));
		if (strcmp(entry->key,"sample_volume") == 0)
			sop_sample_volume(atoi(entry->value));
		if (strcmp(entry->key,"particles") == 0)
			sop_particles(atoi(entry->value));
		if (strcmp(entry->key,"rotation") == 0)
			sop_rotation(atoi(entry->value));
		if (strcmp(entry->key,"direction_flip") == 0)
			sop_direction_flip(atoi(entry->value));
		if (strcmp(entry->key,"show_names") == 0)
			sop_show_names(atoi(entry->value));
		if (strcmp(entry->key,"show_map") == 0)
			sop_show_map(atoi(entry->value));
		if (strcmp(entry->key,"global_chat") == 0)
			sop_global_chat(atoi(entry->value));
		if (strcmp(entry->key,"game_options") == 0)
			sop_game_options(atoi(entry->value));
		if (strcmp(entry->key,"game_hares") == 0)
			sop_game_hares(atoi(entry->value));
		if (strcmp(entry->key,"game_seconds") == 0)
			sop_game_seconds(atoi(entry->value));
		if (strcmp(entry->key,"first_game") == 0)
			sop_first_game(atoi(entry->value));
		if (strcmp(entry->key,"sprite") == 0)
			sop_sprite(atoi(entry->value));
		if (strcmp(entry->key,"update_server") == 0)
			sop_update_server(atoi(entry->value));
		entry = entry->next;
	}
	spNetC4AProfilePointer profile;
	if (found_username == 0 && (profile = spNetC4AGetProfile()))
	{
		 sop_username(profile->longname);
		 spNetC4AFreeProfile(profile);
	}
	sop_music_volume(gop_music_volume());
	sop_sample_volume(gop_sample_volume());
	spConfigFree(conf);
	if (gop_update_server() < 2)
	{
		if ( strcmp( gop_server(), "ziz.gp2x.de/hase.php" ) == 0 ||
			 strcmp( gop_server(), "ziz.myftp.info/hase/hase.php" ) == 0 )
		{
			sop_update_server( 2 );
			sop_server("hase.schmidt-matthes.de/hase.php");
			save_options();
		}
	}
}

void save_options()
{
	spConfigPointer conf = spConfigRead("config.ini","hase");
	spConfigSetInt(conf,"circle",op_circle);
	spConfigSetInt(conf,"music_volume",op_music_volume);
	spConfigSetInt(conf,"sample_volume",op_sample_volume);
	spConfigSetInt(conf,"particles",op_particles);
	spConfigSetInt(conf,"rotation",op_rotation);
	spConfigSetInt(conf,"direction_flip",op_direction_flip);
	sprintf(spConfigGetString(conf,"server",""),"%s",op_server);
	sprintf(spConfigGetString(conf,"username",""),"%s",op_username);
	spConfigSetInt(conf,"show_names",op_show_names);
	spConfigSetInt(conf,"show_map",op_show_map);
	spConfigSetInt(conf,"global_chat",op_global_chat);
	spConfigSetInt(conf,"game_options",op_game_options);
	spConfigSetInt(conf,"game_seconds",op_game_seconds);
	spConfigSetInt(conf,"game_hares",op_game_hares);
	spConfigSetInt(conf,"first_game",op_first_game);
	spConfigSetInt(conf,"sprite",op_sprite);
	spConfigSetInt(conf,"update_server",op_update_server);
	spConfigWrite(conf);
	spConfigFree(conf);
}

int options_feedback( pWindow window, pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_LEFT:
			switch (elem->reference)
			{
				case 1:
					sop_circle(1-gop_circle());
					break;
				case 2:
					sop_music_volume(gop_music_volume()-1);
					break;
				case 3:
					sop_sample_volume(gop_sample_volume()-1);
					break;
				case 4:
					sop_particles(gop_particles()-1);
					break;
				case 5:
					sop_rotation(1-gop_rotation());
					options_feedback(window,elem->next,WN_ACT_UPDATE);
					break;
				case 6:
					sop_direction_flip(1-gop_direction_flip());
					break;
				case 7:
					sop_show_names(1-gop_show_names());
					break;
				case 8:
					sop_show_map(1-gop_show_map());
					break;
				case 9:
					sop_global_chat(1-gop_global_chat());
					break;
			}
			break;
		case WN_ACT_RIGHT:
			switch (elem->reference)
			{
				case 1:
					sop_circle(1-gop_circle());
					break;
				case 2:
					sop_music_volume(gop_music_volume()+1);
					break;
				case 3:
					sop_sample_volume(gop_sample_volume()+1);
					break;
				case 4:
					sop_particles(gop_particles()+1);
					break;
				case 5:
					sop_rotation(1-gop_rotation());
					options_feedback(window,elem->next,WN_ACT_UPDATE);
					break;
				case 6:
					sop_direction_flip(1-gop_direction_flip());
					break;
				case 7:
					sop_show_names(1-gop_show_names());
					break;
				case 8:
					sop_show_map(1-gop_show_map());
					break;
				case 9:
					sop_global_chat(1-gop_global_chat());
					break;
			}
			break;
	}
	if (elem->type != 2 || action == WN_ACT_UPDATE)
	switch (elem->reference)
	{
		case 1:
			if (gop_circle())
				sprintf(elem->text,"Mark active hare: Yes");
			else
				sprintf(elem->text,"Mark active hare: No");
			break;
		case 2:
			sprintf(elem->text,"Music volume: %i%%",gop_music_volume()*100/SP_VOLUME_MAX >> VOLUME_SHIFT);
			break;
		case 3:
			sprintf(elem->text,"Effect volume: %i%%",gop_sample_volume()*100/SP_VOLUME_MAX >> VOLUME_SHIFT);
			break;
		case 4:
			switch (gop_particles())
			{
				case 1:
					sprintf(elem->text,"Particles: Very much");
					break;
				case 2:
					sprintf(elem->text,"Particles: Much");
					break;
				case 3:
					sprintf(elem->text,"Particles: Not so much");
					break;
				case 4:
					sprintf(elem->text,"Particles: No");
					break;
			}
			break;
		case 5:
			if (gop_rotation())
				sprintf(elem->text,"Rotating level: Yes");
			else
				sprintf(elem->text,"Rotating level: No");
			break;
		case 6:
			if (gop_rotation())
				sprintf(elem->text,"(Setting deactivated)");
			else
			if (gop_direction_flip())
				sprintf(elem->text,"Flip direction controls: Yes");
			else
				sprintf(elem->text,"Flip direction controls: No");
			break;
		case 7:
			if (gop_show_names())
				sprintf(elem->text,"Show names: Yes");
			else
				sprintf(elem->text,"Show names: No");
			break;
		case 8:
			if (gop_show_map())
				sprintf(elem->text,"Show map: Yes");
			else
				sprintf(elem->text,"Show map: No");
			break;
		case 9:
			if (gop_global_chat())
				sprintf(elem->text,"Show global chat ingame: Yes");
			else
				sprintf(elem->text,"Show global chat ingame: No");
			break;
		case 10:
			sprintf(elem->text,"Ingame button mapping");
			break;
		case 11:
			sprintf(elem->text,"Menu button mapping");
			break;
		case 12:
			sprintf(elem->text,"Quit game");
			break;
	}
	return 0;
}

int options_window(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int quit)
{
	pWindow window = create_window(options_feedback,font,"Options");
	window->only_ok = 1;
	add_window_element(window,0,1);
	add_window_element(window,2,2);
	add_window_element(window,2,3);
	add_window_element(window,0,4);
	add_window_element(window,0,5);
	add_window_element(window,0,6);
	add_window_element(window,0,7);
	add_window_element(window,0,8);
	add_window_element(window,0,9);
	add_window_element(window,-1,10);
	add_window_element(window,-1,11);
	if (quit)
		add_window_element(window,-1,12);
	int con = 1;
	int ret = 0;
	while (con)
	{
		con = 0;
		int res = modal_window(window,resize);
		if (window->selection == 9 && res == 1)
		{
			con = 1;
			mapping_window(font,resize,1);
		}
		if (window->selection == 10 && res == 1)
		{
			con = 1;
			mapping_window(font,resize,0);
		}
		if (window->selection == 11 && res == 1)
			ret = 1;
	}
	delete_window(window);
	save_options();
	return ret;
}
