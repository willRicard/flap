#version 450 core
layout(location = 0) in vec4 in_vertex;

layout(location = 0) out vec2 frag_texcoord;

void main() {
  gl_Position = vec4(in_vertex.xy, 0.0f, 1.0f);
  frag_texcoord = in_vertex.zw;
}
