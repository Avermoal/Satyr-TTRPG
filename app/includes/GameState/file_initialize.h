#ifndef GAMESTATE_FILE_INITIALIZE_H
#define GAMESTATE_FILE_INITIALIZE_H

/*Create all needed dir*/
void makedir(const char* path);

void campaign_dir_init(const char* name);

void make_scene_dir(const char* name, int snum);

#endif/*GAMESTATE_FILE_INITIALIZE_H*/
