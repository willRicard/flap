#version 450

layout(push_constant) uniform PushConstants { vec3 color; }
push_constants;

out layout(location = 0) vec4 out_color;

void main() { out_color = vec4(push_constants.color, 1.0); }
