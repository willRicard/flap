#include "sprite_impl.h"

#include <string.h>

#include "assets_gl.h"
#include "window.h"

static GLuint texture = 0;
static GLint location_texture = 0;

static GLuint program = 0;

static GLuint vao = 0;

static GLuint buffers[2] = {0};

void sprite_gl_init() {
  // Detect which shader to use: OpenGL or OpenGL ES / WebGL
  const GLubyte *version = glGetString(GL_VERSION);

  const char *vertex_shader_source = NULL;
  const char *fragment_shader_source = NULL;

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
  vertex_shader_source = "shaders/sprite_es.vert";
  fragment_shader_source = "shaders/sprite_es.frag";
#else
  vertex_shader_source = "shaders/sprite_gl.vert";
  fragment_shader_source = "shaders/sprite_gl.frag";
#endif

  GLuint vertex_shader =
      assets_gl_create_shader(GL_VERTEX_SHADER, vertex_shader_source);
  GLuint fragment_shader =
      assets_gl_create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

  program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  glLinkProgram(program);
  assets_gl_check_program(program);

  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glUseProgram(program);

  texture = assets_gl_create_texture("images/atlas.png");
  location_texture = glGetUniformLocation(program, "texture_sampler");

  if (glad_glGenVertexArrays) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }

  glGenBuffers(2, buffers);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
}

void sprite_gl_quit() {
  glDeleteProgram(program);

  if (glad_glDeleteVertexArrays) {
    glDeleteVertexArrays(1, &vao);
  }

  glDeleteBuffers(2, buffers);
  glDeleteTextures(1, &texture);
}

void sprite_update() {
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  if (!glad_glGenVertexArrays) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  }

  glUniform1i(location_texture, 0);

  glDrawElements(GL_TRIANGLES, kIndicesPerSprite * kNumSprites,
                 GL_UNSIGNED_SHORT, 0);

  if (!glad_glGenVertexArrays) {
    glDisableVertexAttribArray(0);
  }
}
