#include "rect.h"

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "flap.h"
#include "shader.h"

// clang-format off
static const char *vertex_shader_source = GLSL(
in vec4 pos;
in vec4 size;
in vec4 color;

out vec2 geom_size;
out vec3 geom_color;

void main() {
	gl_Position = vec4(pos.xy, 0.0, 1.0);
	geom_size = size.xy;
	geom_color = color.xyz;
});

static const char *fragment_shader_source = GLSL(
in vec3 frag_color;
out vec4 out_color;

void main() {
	out_color = vec4(frag_color, 1.f);
});

static const char *geometry_shader_source = GLSL(
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 geom_size[];
in vec3 geom_color[];

out vec3 frag_color;

void main() {
	frag_color = geom_color[0];

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(geom_size[0].x, 0.0, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(0.0, geom_size[0].y, 0.0, 0.0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(geom_size[0], 0.0, 0.0);
	EmitVertex();

	EndPrimitive();
});
// clang-format on

static flap_Shader *shader = NULL;
static GLuint vao = 0, vbo = 0, count = 0;
static flap_Rect *buffer = NULL;

void flap_rect_init() {
  shader = flap_shader_new(vertex_shader_source, fragment_shader_source,
                           geometry_shader_source);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        (const void *)(4 * sizeof(float)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                        (const void *)(8 * sizeof(float)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  buffer = (flap_Rect *)malloc(9 * sizeof(flap_Rect));
  memset(buffer, 0, 9 * sizeof(flap_Rect));
}

void flap_rect_quit() {
  glDeleteBuffers(1, &vbo);

  glDeleteVertexArrays(1, &vao);

  flap_shader_free(shader);
}

flap_Rect *flap_rect_new() {
  flap_Rect *rect = &buffer[count];

  count++;

  return rect;
}

void flap_rect_draw() {
  flap_shader_use(shader);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, count * sizeof(flap_Rect), buffer,
               GL_DYNAMIC_DRAW);

  glDrawArrays(GL_POINTS, 0, count);
}
