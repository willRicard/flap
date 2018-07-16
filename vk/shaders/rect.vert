#version 440 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPos;

void main() {
  gl_Position = vec4(inPos.xy, 0.0f, 1.0f);
  gl_Position *= vec4(1.0f, -1.0f, 1.0f, 1.0f); // Flip Y component
}
