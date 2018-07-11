#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"

static void compile_shader(GLuint shader, const char *source) {
  size_t len = strlen(source);
  glShaderSource(shader, 1, &source, (const GLint *)&len);

  glCompileShader(shader);

  GLint err = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &err);

  if (err != GL_TRUE) {
    GLint err_size;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &err_size);
    if (err == 0) {
      fputs("No info log was provided.", stderr);
      exit(1);
    }

    char *buf = NULL;
    buf = (char *)malloc(err_size * sizeof(char));
    if (buf == NULL) {
      fputs("Memory allocation failed!", stderr);
      exit(1);
    }

    glGetShaderInfoLog(shader, err_size, &err_size, buf);

    puts(buf);

    free(buf);

    exit(1);
  }
}

flap_Shader *flap_shader_new(const char *vertex_shader_source,
                             const char *fragment_shader_source) {
  flap_Shader *shader = (flap_Shader *)malloc(sizeof(flap_Shader));
  shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  shader->program = glCreateProgram();

  compile_shader(shader->vertex_shader, vertex_shader_source);
  compile_shader(shader->fragment_shader, fragment_shader_source);

  glAttachShader(shader->program, shader->vertex_shader);
  glAttachShader(shader->program, shader->fragment_shader);

  glLinkProgram(shader->program);

  GLint err;
  glGetProgramiv(shader->program, GL_LINK_STATUS, &err);
  if (err != GL_TRUE) {
    glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &err);

    if (err == 0) {
      fputs("No info log was provided.", stderr);
      exit(1);
    }

    char *buf = (char *)malloc(err * sizeof(char));
    if (buf == NULL) {
      fputs("Memory allocation failed.", stderr);
      exit(1);
    }

    glGetProgramInfoLog(shader->program, err, &err, buf);

    puts(buf);

    free(buf);

    exit(1);
  }

  return shader;
}

void flap_shader_free(flap_Shader *shader) {
  glDeleteShader(shader->vertex_shader);
  glDeleteShader(shader->fragment_shader);
  glDeleteProgram(shader->program);
  free(shader);
}
