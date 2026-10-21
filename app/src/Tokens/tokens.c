#include "Tokens/tokens.h"

#include <string.h>

void settoken(struct token* tok, int32_t type, int32_t id, const char* path)
{
  if(!tok){
    return;
  }
  tok->type = type;
  tok->id = id;
  tok->path = strdup(path);
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
