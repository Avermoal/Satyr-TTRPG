#version 320 es

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

uniform mat4 u_proj;

out vec2 v_uv;
out vec4 v_color;

void main()
{
    v_uv = a_uv;

    v_color = a_color;

    /*
      Если вдруг текстуры/атлас отображаются перевернутыми по вертикали,
      попробуйте раскомментировать эту строку:
    */
    // v_uv = vec2(a_uv.x, 1.0 - a_uv.y);

    gl_Position = u_proj * vec4(a_pos, 0.0, 1.0);
}
