#include "Map/texture_atlas.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#include <dirent.h>
#include <sys/stat.h>

#elif defined(_WIN32)

#include <windows.h>

#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "Map/map_data.h"

static int32_t next_pow2_int(int32_t value);

static uint64_t estimatearea(const img_info* images, int32_t count, int32_t padding);

static int32_t estimate_initial_side(uint64_t area, int32_t min_size, int32_t max_size);

static int32_t max_required_width(const img_info* images, int32_t count, int32_t padding);

static bool growwidth(shelfcontext* ctx, int32_t required_w);
static bool growheight(shelfcontext* ctx, int32_t required_h);

static bool shelfplace(shelfcontext* ctx, int32_t w, int32_t h, rect* out);

static int32_t compare_by_height_desc(const void* a, const void* b);

static bool pack_all(const img_info* images, int32_t count,
                     int32_t padding, int32_t maxsize,
                     int32_t start_w, int32_t start_h,
                     shelfcontext* outctx, rect* placed);

static void blit_image_rgba(uint8_t* canvas, int32_t canvas_w,
                            int32_t destx, int32_t desty,
                            const uint8_t* pixels,
                            int32_t w, int32_t h);

static int32_t get_names_of_textures(const char* dir_path, char*** out_files);

void get_path_to_atlas(const char* textures_path, const char* atlas_dir_path, int32_t lrnum, struct layer* l, char** atlas_path)
{
  /*atlas_path = atlas_dir_path + "/lrnum.png"*/
  *atlas_path = nullptr;
  char** full_path = nullptr;
  int path_len = snprintf(NULL, 0 , "%s/%d.png", atlas_dir_path, lrnum);
  *atlas_path = (char*)malloc((size_t)path_len + 1);
  snprintf(*atlas_path, (size_t)path_len + 1, "%s/%d.png", atlas_dir_path, lrnum);
  /*Get atlas*/
  char** filenames = nullptr;
  int32_t tex_count = get_names_of_textures(textures_path, &filenames);
  img_info* img = (img_info*)malloc(tex_count * sizeof(img_info));
  rect* placed = (rect*)malloc(tex_count * sizeof(rect));
    /*Settings*/
  const int32_t padding = 2;
  const int32_t max_atlas_size = 8192;
  int32_t tex_path_length = strlen(textures_path);
    /*Get images sizes*/
  for(int i = 0; i < tex_count; ++i){
    int w = 0;
    int h = 0;
    int channels = 0;
    char path_to_tex[PATH_MAX];
    strncat(path_to_tex, textures_path, tex_path_length);
    path_to_tex[tex_path_length] = '/';
    path_to_tex[tex_path_length + 1] = '\0';
    strncat(path_to_tex, filenames[i], strlen(filenames[i]));
    stbi_info(path_to_tex, &w, &h, &channels);
    img[i].path = path_to_tex;/*WARRNING*/
    img[i].w = w;
    img[i].h = h;
    img[i].originalindex = i;
  }
    /*Sort*/
  qsort(img, tex_count, sizeof(img_info), compare_by_height_desc);
    /*Prepare start atlas sizes*/
  uint64_t estimated_area = estimatearea(img, tex_count, padding);
  int side = estimate_initial_side(estimated_area, 256, max_atlas_size);
  int required_width = max_required_width(img, tex_count, padding);
  int start_width = side;
  if(start_width < required_width){
    start_width = required_width;
  }
  start_width = next_pow2_int(start_width);
  if(start_width > max_atlas_size){
    start_width = max_atlas_size;
  }
  int start_height = side;
  if(start_height < 256){
    start_height = 256;
  }
  if(start_height > max_atlas_size){
    start_height = max_atlas_size;
  }
    /*Packaing*/
  shelfcontext finalctx;
  pack_all(img, tex_count, padding, max_atlas_size, start_width, start_height, &finalctx, placed);
  int tight_width = next_pow2_int(finalctx.used_w);
  int tight_height = next_pow2_int(finalctx.used_h);
  if(tight_width < 1){
    tight_width = 1;
  }
  if(tight_height < 1){
    tight_height = 1;
  }
  if(tight_width > max_atlas_size){
    tight_width = max_atlas_size;
  }
  if(tight_height > max_atlas_size){
    tight_height = max_atlas_size;
  }
  if(tight_width < finalctx.w){
    finalctx.w = tight_width;
  }
  if(tight_height < finalctx.h){
    finalctx.h = tight_height;
  }
    /*Create canvas*/
  size_t canvas_bytes = (size_t)finalctx.w * (size_t)finalctx.h * 4;

  uint8_t* canvas = (uint8_t*)calloc(1, canvas_bytes);

  if(!canvas){
    free(img);
    free(placed);
    return;
  }
    /*Loading image to atlas*/
  for(int i = 0; i < tex_count; i++){
    int w = 0;
    int h = 0;
    int channels = 0;
    
    char path_to_tex[PATH_MAX];
    strncat(path_to_tex, textures_path, tex_path_length);
    path_to_tex[tex_path_length] = '/';
    path_to_tex[tex_path_length + 1] = '\0';
    strncat(path_to_tex, filenames[i], strlen(filenames[i]));

    uint8_t* pixels = stbi_load(path_to_tex, &w, &h, &channels, 4);
    if(!pixels){
      free(canvas);
      free(img);
      free(placed);
      return;
    }

    if(w != placed[i].w || h != placed[i].h){
      stbi_image_free(pixels);
      free(canvas);
      free(img);
      free(placed);
      return;
    }

    blit_image_rgba(
      canvas,
      finalctx.w,
      placed[i].x,
      placed[i].y,
      pixels,
      w,
      h);
    stbi_image_free(pixels);
  }
    /*Save PNG*/
  int write_ok = stbi_write_png(*atlas_path, finalctx.w, finalctx.h, 4, canvas, finalctx.w * 4);
  if(!write_ok){
    fprintf(stderr, "PNG is not saved: %s\n", *atlas_path);
    free(canvas);
    free(img);
    free(placed);
    return;
  }
  printf("Atlas saved: %s (%dx%d)\n",
           *atlas_path,
           finalctx.w,
           finalctx.h);
    /*UV, sizes, and world coord*/
  for(int i = 0; i < tex_count; i++){
    float u0 = (float)placed[i].x / (float)finalctx.w;
    float v0 = (float)placed[i].y / (float)finalctx.h;

    float u1 = (float)(placed[i].x + placed[i].w) / (float)finalctx.w;
    float v1 = (float)(placed[i].y + placed[i].h) / (float)finalctx.h;

    rectset(&l->rects[i], 0, 0, (float)placed[i].w, (float)placed[i].h,
            u0, v0, u1, v1, 1.0f, 1.0f, 1.0f, 1.0f);
  }
  /*Free*/
  free(canvas);
  free(img);
  free(placed);
}

static int32_t next_pow2_int(int32_t value)
{
  if(value <= 1){
    return 1;
  }
  uint32_t v = (uint32_t)value;
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return (int32_t)v;
}

static uint64_t estimatearea(const img_info* images, int32_t count, int32_t padding)
{
  uint64_t area = 0;
  for(int i = 0; i < count; ++i){
    uint64_t w = (uint64_t)(images[i].w + padding * 2);
    uint64_t h = (uint64_t)(images[i].h + padding * 2);
    area += w * h;
  }
  return area;
}

static int32_t estimate_initial_side(uint64_t area, int32_t min_size, int32_t max_size)
{
  uint64_t target = area + area / 4; // запас 25%
  uint64_t side = (uint64_t)min_size;

  while (side * side < target) {
    side *= 2;
    if(side > (uint64_t)max_size){
      side = (uint64_t)max_size;
      break;
    }
  }

  if(side > (uint64_t)max_size){
    side = (uint64_t)max_size;
  }
  return (int32_t)side;
}

static int32_t max_required_width(const img_info* images, int32_t count, int32_t padding)
{
  int result = 0;

  for (int i = 0; i < count; i++) {
    int w = images[i].w + padding * 2;
    if(w > result){
      result = w;
    }
  }
  return result;
}

static bool growwidth(shelfcontext* ctx, int32_t required_w)
{
  if(required_w <= ctx->w){
    return true;
  }
  int new_width = ctx->w;
  while(new_width < required_w){
    if (new_width > ctx->maxsize / 2){
      new_width = ctx->maxsize;
      break;
    }
    new_width *= 2;
  }
  if(new_width > ctx->maxsize){
    new_width = ctx->maxsize;
  }
  if(new_width < required_w){
    return false;
  }
  ctx->w = new_width;
  return true;
}

static bool growheight(shelfcontext* ctx, int32_t required_h)
{
  if(required_h <= ctx->h){
    return true;
  }
  int new_height = ctx->h;
  while(new_height < required_h){
    if (new_height > ctx->maxsize / 2) {
      new_height = ctx->maxsize;
      break;
    }
    new_height *= 2;
  }
  if(new_height > ctx->maxsize){
    new_height = ctx->maxsize;
  }
  if(new_height < required_h){
    return false;
  }
  ctx->h = new_height;
  return true;
}

static bool shelfplace(shelfcontext* ctx, int32_t w, int32_t h, rect* out)
{
  if(w <= 0 || h <= 0){
    return false;
  }
  int required_width = w + ctx->padding * 2;
  if(!growwidth(ctx, required_width)){
    return false;
  }
  if(ctx->cursorx + w + ctx->padding > ctx->w){
    ctx->cursorx = ctx->padding;
    ctx->cursory += ctx->shelf_h + ctx->padding;
    ctx->shelf_h = 0;
  }

  int required_height = ctx->cursory + h + ctx->padding;

  if(!growheight(ctx, required_height)) {
    return false;
  }

  out->x = ctx->cursorx;
  out->y = ctx->cursory;
  out->w = w;
  out->h = h;

  ctx->cursorx += w + ctx->padding;

  if(h > ctx->shelf_h){
    ctx->shelf_h = h;
  }
  if(ctx->cursorx > ctx->used_w){
    ctx->used_w = ctx->cursorx;
  }

  int bottom = ctx->cursory + ctx->shelf_h + ctx->padding;
  if(bottom > ctx->used_h){
    ctx->used_h = bottom;
  }
  return true;
}

static int32_t compare_by_height_desc(const void* a, const void* b)
{
  const img_info* ia = (const img_info*)a;
  const img_info* ib = (const img_info*)b;

  if(ib->h != ia->h){
    return (ib->h > ia->h) ? 1 : -1;
  }

  if(ib->w != ia->w){
    return (ib->w > ia->w) ? 1 : -1;
  }

  int64_t area_a = (int64_t)ia->w * ia->h;
  int64_t area_b = (int64_t)ib->w * ib->h;

  if(area_b > area_a){
    return 1;
  }

  if(area_b < area_a){
    return -1;
  }
  return 0;
}

static bool pack_all(const img_info* images, int32_t count,
                     int32_t padding, int32_t maxsize,
                     int32_t start_w, int32_t start_h,
                     shelfcontext* outctx, rect* placed)
{
  int attempt_width = start_w;

  while(attempt_width <= maxsize){
    shelfcontext ctx;

    ctx.w = attempt_width;
    ctx.h = start_h;

    ctx.cursorx = padding;
    ctx.cursory = padding;

    ctx.shelf_h = 0;

    ctx.used_w = padding;
    ctx.used_h = padding;

    ctx.padding = padding;
    ctx.maxsize = maxsize;

    bool ok = true;

    for(int i = 0; i < count; i++){
      rect r;

      if(!shelfplace(&ctx, images[i].w, images[i].h, &r)){
        ok = false;
        break;
      }
      placed[images[i].originalindex] = r;
    }

    if(ok){
      *outctx = ctx;
      return true;
    }

    if(attempt_width >= maxsize){
      break;
    }

    if(attempt_width > maxsize / 2){
      attempt_width = maxsize;
    }else{
      attempt_width *= 2;
    }
  }
  return false;
}

static void blit_image_rgba(uint8_t* canvas, int32_t canvas_w,
                            int32_t destx, int32_t desty,
                            const uint8_t* pixels,
                            int32_t w, int32_t h)
{
  for(int y = 0; y < h; y++){
    size_t dst_offset = ((size_t)(desty + y) * (size_t)canvas_w + (size_t)destx) * 4;
    size_t src_offset = (size_t)y * (size_t)w * 4;
    memcpy(canvas + dst_offset, pixels + src_offset, (size_t)w * 4);
  }
}

static int32_t get_names_of_textures(const char* dir_path, char*** out_path)
{
  int32_t count = 0;

  #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

  DIR* dir = opendir(dir_path);
  if(!dir){
    perror("CAN NOT OPEN DIR WITH TEXTURES\n");
    return -1;
  }
  int capacity = 10;

  *out_path = (char **)malloc(capacity * sizeof(char *));
  if(!*out_path){
    closedir(dir);
    return -1;
  }

  struct dirent* entry;
  while((entry = readdir(dir)) != NULL){
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
      continue;
    }

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

    struct stat st;
    if(stat(full_path, &st) == 0 && S_ISREG(st.st_mode)){
      if(count >= capacity){
        capacity *= 2;
        char **temp = (char **)realloc(*out_path, capacity * sizeof(char *));
        if(!temp){
          for(int i = 0; i < count; i++){
            free((*out_path)[i]);
          }
          free(*out_path);
          *out_path = NULL;
          closedir(dir);
          return -1;
        }
        *out_path = temp;
      }
      (*out_path)[count] = strdup(entry->d_name);
      count++;
    }
  }
  closedir(dir);

  #elif defined(_WIN32)

  char search_path[MAX_PATH];
  size_t len = strlen(dir_path);

  if(len > 0 && (dir_path[len - 1] == '\\' || dir_path[len - 1] == '/')){
    snprintf(search_path, MAX_PATH, "%s*", dir_path);
  }else{
    snprintf(search_path, MAX_PATH, "%s\\*", dir_path);
  }

  WIN32_FIND_DATAA find_data;
  HANDLE hFind = FindFirstFileA(search_path, &find_data);

  if(hFind == INVALID_HANDLE_VALUE){
    printf("CAN NOT OPEN TEXTURES DIR: %lu\n", GetLastError());
    return -1;
  }

  int capacity = 10;

  *out_path = (char **)malloc(capacity * sizeof(char *));
  if(!*out_path){
    FindClose(hFind);
    return -1;
  }
  do{
    if(strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0){
      continue;
    }
    if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
      continue;
    }
    if(count >= capacity){
      capacity *= 2;
      char **temp = (char **)realloc(*out_path, capacity * sizeof(char *));
      if(!temp){
        for(int i = 0; i < count; i++){
          free((*out_path)[i]);
        }
          free(*out_path);
          *out_path = NULL;
          FindClose(hFind);
          return -1;
      }
      *out_path = temp;
    }
    (*out_path)[count] = strdup(find_data.cFileName);
    count++;
  }while(FindNextFileA(hFind, &find_data) != 0);

  FindClose(hFind);

  #endif

  return count;
}
