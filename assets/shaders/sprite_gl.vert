#version 330 core
in vec4 in_vertex;

out vec2 frag_texcoord;

void main() {
  gl_Position = vec4(in_vertex.xy, 0.0f, 1.0f);
  gl_Position.y *= -1.0;
  frag_texcoord = in_vertex.zw;
}
