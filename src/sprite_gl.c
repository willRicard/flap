#include "sprite.h"

#include <string.h>

#include "assets_gl.h"
#include "flap.h"
#include "window.h"

static const float kTextureWidth = 128.F;
static const float kTextureHeight = 32.F;

static unsigned int count = 0;

static Sprite vertices[1 + 4 * FLAP_NUM_PIPES];

static const unsigned short indices[] = {
    0,   1,   2,   2,   0,   3,   4,   5,   6,   6,   4,   7,   8,   9,   10,
    10,  8,   11,  12,  13,  14,  14,  12,  15,  16,  17,  18,  18,  16,  19,
    20,  21,  22,  22,  20,  23,  24,  25,  26,  26,  24,  27,  28,  29,  30,
    30,  28,  31,  32,  33,  34,  34,  32,  35,  36,  37,  38,  38,  36,  39,
    40,  41,  42,  42,  40,  43,  44,  45,  46,  46,  44,  47,  48,  49,  50,
    50,  48,  51,  52,  53,  54,  54,  52,  55,  56,  57,  58,  58,  56,  59,
    60,  61,  62,  62,  60,  63,  64,  65,  66,  66,  64,  67,  68,  69,  70,
    70,  68,  71,  72,  73,  74,  74,  72,  75,  76,  77,  78,  78,  76,  79,
    80,  81,  82,  82,  80,  83,  84,  85,  86,  86,  84,  87,  88,  89,  90,
    90,  88,  91,  92,  93,  94,  94,  92,  95,  96,  97,  98,  98,  96,  99,
    100, 101, 102, 102, 100, 103, 104, 105, 106, 106, 104, 107, 108, 109, 110,
    110, 108, 111, 112, 113, 114, 114, 112, 115, 116, 117, 118, 118, 116, 119,
    120, 121, 122, 122, 120, 123, 124, 125, 126, 126, 124, 127, 128, 129, 130,
    130, 128, 131, 132, 133, 134, 134, 132, 135, 136, 137, 138, 138, 136, 139,
    140, 141, 142, 142, 140, 143, 144, 145, 146, 146, 144, 147, 148, 149, 150,
    150, 148, 151, 152, 153, 154, 154, 152, 155, 156, 157, 158, 158, 156, 159,
    160, 161, 162, 162, 160, 163, 164, 165, 166, 166, 164, 167,
};

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

  if (strncmp((const char *)version, "OpenGL ES", 9) == 0) {
    vertex_shader_source = "shaders/sprite_es.vert";
    fragment_shader_source = "shaders/sprite_es.frag";
  } else {
    vertex_shader_source = "shaders/sprite_gl.vert";
    fragment_shader_source = "shaders/sprite_gl.frag";
  }

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
  // perf: Since the game only has sprites for now,
  // bind vertex buffer once and for all in `sprite_init`
  //
  // glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  //
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  if (!glad_glGenVertexArrays) {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  }

  glUniform1i(texture, location_texture);

  glDrawElements(GL_TRIANGLES, 6 * (1 + 4 * FLAP_NUM_PIPES), GL_UNSIGNED_SHORT,
                 0);

  if (!glad_glGenVertexArrays) {
    glDisableVertexAttribArray(0);
  }
}

Sprite *sprite_new(float texture_x, float texture_y, float texture_w,
                   float texture_h) {
  Sprite *sprite = &vertices[count++];

  const float left_texcoord = texture_x / kTextureWidth;
  const float top_texcoord = texture_y / kTextureHeight;
  const float right_texcoord = (texture_x + texture_w) / kTextureWidth;
  const float bottom_texcoord = (texture_y + texture_h) / kTextureHeight;

  sprite->vertices[0].tx = left_texcoord;
  sprite->vertices[0].ty = top_texcoord;

  sprite->vertices[1].tx = left_texcoord;
  sprite->vertices[1].ty = bottom_texcoord;

  sprite->vertices[2].tx = right_texcoord;
  sprite->vertices[2].ty = bottom_texcoord;

  sprite->vertices[3].tx = right_texcoord;
  sprite->vertices[3].ty = top_texcoord;

  return sprite;
}
