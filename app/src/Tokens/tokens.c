#include "Tokens/tokens.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

void settoken(struct token* tok, int32_t type, int32_t id, const char* path, const char* cache)
{
  if(!tok){
    return;
  }
  tok->type = type;
  tok->id = id;
  tok->path = path ? strdup(path) : nullptr;
  tok->cache = cache ? strdup(cache) : nullptr;
}

void set_token_UV(struct token* tok, float u0, float v0, float u1, float v1)
{
  if(!tok){
    return;
  }
  tok->u0 = u0;
  tok->v0 = v0;
  tok->u1 = u1;
  tok->v1 = v1;
}

void copytoken(struct token* dst, struct token* src)
{
  dst->type = src->type;
  dst->id = src->id;
  dst->u0 = src->u0;
  dst->v0 = src->v0;
  dst->u1 = src->u1;
  dst->v1 = src->v1;
  dst->path = src->path ? strdup(src->path) : nullptr;
  dst->cache = src->cache ? strdup(src->cache) : nullptr;
}

void set_token_UV_from_token(struct token* dst, struct token* src)
{
  dst->u0 = src->u0;
  dst->v0 = src->v0;
  dst->u1 = src->u1;
  dst->v1 = src->v1;
}

bool savetoken(struct token* t, FILE* state)
{
  int32_t pathlen = strlen(t->path);
  int32_t cachelen = 0;
  if(t->cache){
    cachelen = strlen(t->cache);
  }
  if(fwrite(&t->type, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  if(fwrite(&t->id, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  if(fwrite(&t->u0, sizeof(float), 1, state) < 1){
    return false;
  }
  if(fwrite(&t->v0, sizeof(float), 1, state) < 1){
    return false;
  }
  if(fwrite(&t->u1, sizeof(float), 1, state) < 1){
    return false;
  }
  if(fwrite(&t->v1, sizeof(float), 1, state) < 1){
    return false;
  }
  if(fwrite(&pathlen, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  if(fwrite(&cachelen, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  if(fwrite(t->path, sizeof(char), pathlen, state) < pathlen){
    return false;
  }
  if(fwrite(t->cache, sizeof(char), cachelen, state) < cachelen){
    return false;
  }
  return true;
}

struct token* gettokens(int32_t tnum, FILE* state)
{
  if(tnum <= 0){
    return nullptr;
  }
  int32_t type = 0;
  int32_t id = 0;
  float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
  int32_t pathlen = 0;
  int32_t cachelen = 0;
  char path[512];
  char cache[512];
  struct token* ts = (struct token*)calloc(tnum, sizeof(struct token));
  if(!ts){
    return nullptr;
  }
  for(int32_t i = 0; i < tnum; ++i){
    if(fread(&type, sizeof(int32_t), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&id, sizeof(int32_t), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&u0, sizeof(float), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&v0, sizeof(float), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&u1, sizeof(float), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&v1, sizeof(float), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&pathlen, sizeof(int32_t), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(&cachelen, sizeof(int32_t), 1, state) < 1){
      free(ts);
      return nullptr;
    }
    if(fread(path, sizeof(char), pathlen, state) < pathlen){
      free(ts);
      return nullptr;
    }
    path[pathlen] = '\0';
    if(fread(cache, sizeof(char), cachelen, state) < cachelen){
      free(ts);
      return nullptr;
    }
    cache[cachelen] = '\0';
    if(cachelen == 0){
      settoken(&ts[i], type, id, path, nullptr);
      set_token_UV(&ts[i], u0, v0, u1, v1);
    }else if (cachelen > 0){
      settoken(&ts[i], type, id, path, cache);
      set_token_UV(&ts[i], u0, v0, u1, v1);
    }else{
      free(ts);
      return nullptr;
    }
  }
  return ts;
}
