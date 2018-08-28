#version 440 core
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
  vec3 color; 
} pushConstants;

out layout(location = 0) vec4 outColor;

void main() {
  outColor = vec4(pushConstants.color, 1.0f);
}
