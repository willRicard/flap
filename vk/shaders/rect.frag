#version 440 core
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform pushConstants { vec3 color; }
uniformPushConstants;

out layout(location = 0) vec4 outColor;

void main() { outColor = vec4(uniformPushConstants.color, 1.0f); }
