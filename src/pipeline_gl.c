#include "pipeline_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"

// Build a shader object from GLSL source code.
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

Pipeline pipeline_create(const char *vertex_shader_source,
                         const char *fragment_shader_source) {
  Pipeline pipeline;
  pipeline.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  pipeline.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  pipeline.program = glCreateProgram();

  compile_shader(pipeline.vertex_shader, vertex_shader_source);
  compile_shader(pipeline.fragment_shader, fragment_shader_source);

  glAttachShader(pipeline.program, pipeline.vertex_shader);
  glAttachShader(pipeline.program, pipeline.fragment_shader);

  glLinkProgram(pipeline.program);

  GLint err;
  glGetProgramiv(pipeline.program, GL_LINK_STATUS, &err);
  if (err != GL_TRUE) {
    glGetProgramiv(pipeline.program, GL_INFO_LOG_LENGTH, &err);

    if (err == 0) {
      fputs("No info log was provided.", stderr);
      exit(1);
    }

    char *buf = (char *)malloc(err * sizeof(char));
    if (buf == NULL) {
      fputs("Memory allocation failed.", stderr);
      exit(1);
    }

    glGetProgramInfoLog(pipeline.program, err, &err, buf);

    puts(buf);

    free(buf);

    exit(1);
  }

  return pipeline;
}

void pipeline_destroy(Pipeline pipeline) {
  glDeleteShader(pipeline.vertex_shader);
  glDeleteShader(pipeline.fragment_shader);
  glDeleteProgram(pipeline.program);
}
