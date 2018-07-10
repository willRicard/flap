#include "rect.h"

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "flap.h"
#include "shader.h"

const float FLAP_BIRD_COLOR[] = {1.f, 1.f, 0.f};
const float FLAP_PIPE_COLOR[] = {0.035f, 0.42f, 0.035f};

// clang-format off
static const char *vertex_shader_source = GLSL(
in vec2 in_pos;
uniform vec3 color;
out vec3 frag_color;
                                               
void main() {
	gl_Position = vec4(in_pos, 0.0, 1.0);
  frag_color = color;
});

static const char *fragment_shader_source = GLSL(
in vec3 frag_color;
out vec4 out_color;

void main() {
	out_color = vec4(frag_color, 1.f);
});
// clang-format on

static flap_Shader *shader = NULL;
static GLuint vao = 0, vbo = 0, count = 0;
static flap_Rect *buffer = NULL;

void flap_rect_init() {
  shader = flap_shader_new(vertex_shader_source, fragment_shader_source);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); // in vec2 in_pos;

  glEnableVertexAttribArray(0);

  buffer = (flap_Rect *)malloc((1 + FLAP_NUM_PIPES * 2) * sizeof(flap_Rect));
  memset(buffer, 0, (1 + FLAP_NUM_PIPES * 2) * sizeof(flap_Rect));
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

float flap_rect_get_x(flap_Rect *rect) { return rect->vertices[0]; }

float flap_rect_get_y(flap_Rect *rect) { return rect->vertices[1]; }

float flap_rect_get_width(flap_Rect *rect) {
  return rect->vertices[2] - rect->vertices[0];
}

float flap_rect_get_height(flap_Rect *rect) {
  return rect->vertices[5] - rect->vertices[1];
}

void flap_rect_set_x(flap_Rect *rect, float x) {
  float dx = x - rect->vertices[0];
  rect->vertices[0] += dx;
  rect->vertices[2] += dx;
  rect->vertices[4] += dx;
  rect->vertices[6] += dx;
  rect->vertices[8] += dx;
  rect->vertices[10] += dx;
}

void flap_rect_set_y(flap_Rect *rect, float y) {
  float dy = y - rect->vertices[1];
  rect->vertices[1] += dy;
  rect->vertices[3] += dy;
  rect->vertices[5] += dy;
  rect->vertices[7] += dy;
  rect->vertices[9] += dy;
  rect->vertices[11] += dy;
}

void flap_rect_set_position(flap_Rect *rect, float x, float y) {
  flap_rect_set_x(rect, x);
  flap_rect_set_y(rect, y);
}

void flap_rect_move(flap_Rect *rect, float x, float y) {
  flap_rect_set_x(rect, flap_rect_get_x(rect) + x);
  flap_rect_set_y(rect, flap_rect_get_y(rect) + y);
}

void flap_rect_set_width(flap_Rect *rect, float width) {
  rect->vertices[2] = rect->vertices[0] + width;
  rect->vertices[8] = rect->vertices[0] + width;
  rect->vertices[10] = rect->vertices[0] + width;
}

void flap_rect_set_height(flap_Rect *rect, float height) {
  rect->vertices[5] = rect->vertices[1] + height;
  rect->vertices[7] = rect->vertices[1] + height;
  rect->vertices[11] = rect->vertices[1] + height;
}

void flap_rect_set_size(flap_Rect *rect, float width, float height) {
  flap_rect_set_width(rect, width);
  flap_rect_set_height(rect, height);
}

void flap_rect_draw() {
  glUseProgram(shader->program);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, count * sizeof(flap_Rect), buffer,
               GL_DYNAMIC_DRAW);

  const GLuint color_location = glGetUniformLocation(shader->program, "color");

  glUniform3fv(color_location, 1, FLAP_BIRD_COLOR);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glUniform3fv(color_location, 1, FLAP_PIPE_COLOR);

  glDrawArrays(GL_TRIANGLES, 6, FLAP_NUM_PIPES * 6);
}
