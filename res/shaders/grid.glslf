#version 320 es
precision mediump float;

in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_tex;

out vec4 frag_color;

void main()
{
    vec4 tex = texture(u_tex, v_uv);

    frag_color = tex * v_color;
}
