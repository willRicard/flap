#include "pipeline_gl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flap.h"

static void compileShader(GLuint shader, const char *source) {
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

flapPipeline flapPipelineCreate(const char *vertexShaderSource,
                                const char *fragmentShaderSource) {
  flapPipeline shader;
  shader.vertexShader = glCreateShader(GL_VERTEX_SHADER);
  shader.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  shader.program = glCreateProgram();

  compileShader(shader.vertexShader, vertexShaderSource);
  compileShader(shader.fragmentShader, fragmentShaderSource);

  glAttachShader(shader.program, shader.vertexShader);
  glAttachShader(shader.program, shader.fragmentShader);

  glLinkProgram(shader.program);

  GLint err;
  glGetProgramiv(shader.program, GL_LINK_STATUS, &err);
  if (err != GL_TRUE) {
    glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &err);

    if (err == 0) {
      fputs("No info log was provided.", stderr);
      exit(1);
    }

    char *buf = (char *)malloc(err * sizeof(char));
    if (buf == NULL) {
      fputs("Memory allocation failed.", stderr);
      exit(1);
    }

    glGetProgramInfoLog(shader.program, err, &err, buf);

    puts(buf);

    free(buf);

    exit(1);
  }

  return shader;
}

void flapPipelineDestroy(flapPipeline shader) {
  glDeleteShader(shader.vertexShader);
  glDeleteShader(shader.fragmentShader);
  glDeleteProgram(shader.program);
}
