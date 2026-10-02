#include "GameState/game_state.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)

#include <direct.h>
#define MKDIR(path) _mkdir(path)
const char PATH_SEP = '\\';

#else

#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
const char PATH_SEP = '/';

#endif

#include "Map/map.h"

void save_game_state(struct gamestate* gstate)
{

}

void makedir(const char* path)
{
  char tmp[256];
  size_t len = 0;
  /*Temporary copy path in the buffer tmp*/
  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);
  for(int i = 0; i < len; ++i){
    if(tmp[i] == '/' || tmp[i] == '\\'){
      tmp[i] = PATH_SEP;
    }
  }
  /*Delete trailing slash*/
  if(tmp[len - 1] == PATH_SEP){
    tmp[len - 1] = '\0';
  }
  /*Make dir*/
  for(char* p = tmp + 1; *p; ++p){
    if(*p == PATH_SEP){
      *p = '\0';
      MKDIR(tmp);
      *p = PATH_SEP;
    }
  }
  MKDIR(tmp);
}
