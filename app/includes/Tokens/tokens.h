#ifndef TOKENS_TOKENS_H
#define TOKENS_TOKENS_H

#include <stdint.h>
#include <stdio.h>

enum{
  TOKEN = 0,
  MAP = 1
};

struct token{
  int32_t type;
  int32_t id;
  float u0, v0, u1, v1;
  char* path;
  char* cache;
};

typedef struct{
  int32_t id;
  int32_t type;
}tokensdata;

void settoken(struct token* tok, int32_t type, int32_t id, const char* path, const char* cache);

void set_token_UV(struct token* tok, float u0, float v0, float u1, float v1);

void copytoken(struct token* dst, struct token* src);

void set_token_UV_from_token(struct token* dst, struct token* src);

bool savetoken(struct token* t, FILE* state);

struct token* gettokens(int32_t tnum, FILE* state);

#endif/*TOKENS_TOKENS_H*/
