#ifndef FLAP_PIPELINE_GL_H_
#define FLAP_PIPELINE_GL_H_
#include "pipeline.h"

#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n" #src

typedef struct Pipeline {
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program;
} Pipeline;

#endif // FLAP_PIPELINE_GL_H_
