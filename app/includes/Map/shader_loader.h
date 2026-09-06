#ifndef MAP_SHADER_LOADER_H
#define MAP_SHADER_LOADER_H

#include <stdint.h>

typedef uint32_t sh_p;

int load_shader_program(sh_p* id);

sh_p create_shader_program(const char* vsh_txt, const char* fsh_txt);
void destroy_shader_program(const sh_p id);

void bind_shader_program(const sh_p id);
void unbind_shader_program(void);

#endif/*MAP_SHADER_LOADER_H*/
