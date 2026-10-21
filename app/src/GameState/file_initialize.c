#include "GameState/file_initialize.h"

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

void campaign_dir_init(const char* name)
{
  const char* saves = "saves";
  const char* atlas = "presets/atlas/1";
  const char* maps = "presets/textures/maps";
  const char* toks = "presets/textures/tokens";
  /*Make it finish*/
  char atlasdir[256];
  char mapsdir[256];
  char toksdir[256];
  snprintf(atlasdir, sizeof(atlasdir), "%s/%s/%s", saves, name, atlas);
  snprintf(mapsdir, sizeof(mapsdir), "%s/%s/%s", saves, name, maps);
  snprintf(toksdir, sizeof(toksdir), "%s/%s/%s", saves, name, toks);
  /*Create*/
  makedir(atlasdir);
  makedir(mapsdir);
  makedir(toksdir);
}

void make_scene_dir(const char* name, int snum)
{
  const char* saves = "saves";
  const char* scene = "presets/atlas";
  char dir[256];
  snprintf(dir, sizeof(dir), "%s/%s/%s/%d", saves, name, scene, snum);
  makedir(dir);
}
