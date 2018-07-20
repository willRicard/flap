#ifndef FLAP_SHADER_H_
#define FLAP_SHADER_H_
#include <GL/glew.h>

#define GLSL(src) "#version 330 core\n" #src

typedef struct flapShader {
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
} flapShader;

flapShader flapShaderCreate(const char *vertexShaderSource,
                            const char *fragmentShaderSource);

void flapShaderDestroy(flapShader shader);

#endif // FLAP_SHADER_H_
