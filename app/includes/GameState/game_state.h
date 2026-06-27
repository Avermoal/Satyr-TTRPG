#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

struct map;

struct gamestate{
  struct map* currentmap;
};

void save_game_state(struct gamestate* gstate);

#endif/*GAMESTATE_GAME_STATE_H*/
