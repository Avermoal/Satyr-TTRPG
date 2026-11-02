#include "Tokens/tokens.h"

#include <string.h>

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

