#version 440 core
#extension GL_ARB_separate_shader_objects : enable

layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec2 geom_sizes[];

void main() {
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(0.0, geom_sizes[0].y, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(geom_sizes[0].x, 0.0, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(geom_sizes[0].xy, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}
