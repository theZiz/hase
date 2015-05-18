#ifndef OPTIONS_H
#define OPTIONS_H

#include <sparrow3d.h>

#define VOLUME_SHIFT 5

int gop_zoom();
int gop_circle();
int gop_music_volume();
int gop_sample_volume();
int gop_particles();
int gop_rotation();
int gop_direction_flip();
char* gop_server();
char* gop_username();
int gop_show_names();
int gop_show_map();

void sop_zoom(int v);
void sop_circle(int v);
void sop_music_volume(int v);
void sop_sample_volume(int v);
void sop_particles(int v);
void sop_rotation(int v);
void sop_direction_flip(int v);
void sop_server(char* server);
void sop_username(char* username);
void sop_show_names(int v);
void sop_show_map(int v);

void load_options();
void save_options();

int options_window(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int quit);

#endif
