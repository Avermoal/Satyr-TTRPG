#version 310 es
precision mediump float;

uniform float u_grid_opacity;

out vec4 FragColor;

void main(void)
{
  FragColor = vec4(0.5f, 0.5f, 0.5f, u_grid_opacity);
}
