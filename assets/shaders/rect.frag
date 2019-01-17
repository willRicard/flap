#version 450

layout(push_constant) uniform PushConstants {
  vec3 color; 
} pushConstants;

out layout(location = 0) vec4 outColor;

void main() {
  outColor = vec4(pushConstants.color, 1.0f);
}
