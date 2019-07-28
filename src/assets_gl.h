#pragma once
#include "assets.h"
#include <glad/glad.h>

GLuint assets_gl_create_shader(GLenum type, const char *file_path);

void assets_gl_check_program(GLuint program);

GLuint assets_gl_create_texture(const char *file_path);
