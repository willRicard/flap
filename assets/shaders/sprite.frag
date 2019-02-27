#version 450
in layout(location = 0) vec2 frag_texcoord;

out layout(location = 0) vec4 out_color;

layout(binding = 0) uniform sampler2D texture_sampler;

void main() {
  out_color = texture(texture_sampler, frag_texcoord);
}
