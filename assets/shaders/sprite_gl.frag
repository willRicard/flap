#version 330 core
in vec2 frag_texcoord;

out vec4 out_color;

uniform sampler2D texture_sampler;

void main() {
  out_color = texture(texture_sampler, frag_texcoord);
}
