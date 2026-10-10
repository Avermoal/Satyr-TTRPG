#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

struct map;

struct gamestate{
  char* gamename;
  struct map* m;
};

void set_game_state(struct gamestate* gstate, struct map* m, const char* gamename);

void save_game_state(struct gamestate* gstate);

struct gamestate* get_game_state(const char* path);

/*Create all needed dir*/
void makedir(const char* path);

#endif/*GAMESTATE_GAME_STATE_H*/
