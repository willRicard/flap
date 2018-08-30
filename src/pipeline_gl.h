#ifndef FLAP_PIPELINE_GL_H_
#define FLAP_PIPELINE_GL_H_
#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n" #src

typedef struct flapPipeline {
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
} flapPipeline;

flapPipeline flapPipelineCreate(const char *vertexShaderSource,
                                const char *fragmentShaderSource);

void flapPipelineDestroy(flapPipeline shader);

#endif // FLAP_PIPELINE_GL_H_
