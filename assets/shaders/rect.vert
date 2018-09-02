#version 440 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_size;

layout(location = 0) out vec2 geom_size;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = vec4(in_pos, 0.0f, 1.0f);
  geom_size = in_size;
}
