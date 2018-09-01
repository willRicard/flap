#include "rect.h"

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "flap.h"
#include "pipeline_gl.h"

// clang-format off
static const char *vertex_shader_source = GLSL(
in vec2 inPos;
uniform vec3 color;
out vec3 fragColor;
                                               
void main() {
	gl_Position = vec4(inPos, 0.0, 1.0);
  fragColor = color;
});

static const char *fragment_shader_source = GLSL(
in vec3 fragColor;
out vec4 outColor;

void main() {
	outColor = vec4(fragColor, 1.f);
});
// clang-format on

static Pipeline pipeline;
static GLuint vao = 0, vbo = 0, ebo = 0;

void rect_init() {
  pipeline = pipeline_create(vertex_shader_source, fragment_shader_source);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               (1 + FLAP_NUM_PIPES * 2) * 6 * sizeof(uint16_t),
               rect_get_indices(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        0); // in vec2 inPos;

  glEnableVertexAttribArray(0);
}

void rect_quit() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);

  glDeleteVertexArrays(1, &vao);

  pipeline_destroy(pipeline);
}

void rect_draw() {
  glUseProgram(pipeline.program);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBufferData(GL_ARRAY_BUFFER, (1 + FLAP_NUM_PIPES * 2) * sizeof(Rect),
               rect_get_vertices(), GL_DYNAMIC_DRAW);

  const GLuint color_location = glGetUniformLocation(pipeline.program, "color");

  glUniform3fv(color_location, 1, FLAP_BIRD_COLOR);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glUniform3fv(color_location, 1, FLAP_PIPE_COLOR);

  glDrawElements(GL_TRIANGLES, FLAP_NUM_PIPES * 2 * 6, GL_UNSIGNED_SHORT,
                 (const void *)(6 * sizeof(uint16_t)));
}
