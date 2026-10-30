#ifndef TOKENS_TOKENS_H
#define TOKENS_TOKENS_H

#include <stdint.h>

enum{
  TOKEN = 0,
  MAP = 1
};

struct token{
  int32_t type;
  int32_t id;
  float u0, v0, u1, v1;
  char* path;
};

void settoken(struct token* tok, int32_t type, int32_t id, const char* path);

void set_token_UV(struct token* tok, float u0, float v0, float u1, float v1);

void set_token_UV_from_token(struct token* dst, struct token* src);

#endif/*TOKENS_TOKENS_H*/
