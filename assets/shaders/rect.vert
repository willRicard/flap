#version 450 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_size;

layout(location = 0) out vec2 geom_size;

void main() {
  gl_Position = vec4(in_pos, 0.0f, 1.0f);
  geom_size = in_size;
}
