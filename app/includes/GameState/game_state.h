#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

struct map;

struct gamestate{
  struct map* currentmap;
};

void save_game_state(struct gamestate* gstate);

void makedir(const char* path);

#endif/*GAMESTATE_GAME_STATE_H*/
