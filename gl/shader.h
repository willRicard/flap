#ifndef FLAP_SHADER_H_
#define FLAP_SHADER_H_
#include <GL/glew.h>

typedef struct flapShader {
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
} flapShader;

flapShader *flapShaderNew(const char *vertexShaderSource,
                          const char *fragmentShaderSource);

void flapShaderFree(flapShader *shader);

#endif // FLAP_SHADER_H_