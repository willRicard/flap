#include "assets_gl.h"

#include <stdio.h>

#include "window.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "stb_image.h"

GLuint assets_gl_create_shader(GLenum type, const char *file_path) {
  GLuint id = glCreateShader(type);

  size_t size = 0;
  const GLchar *shader_source = assets_read_file(file_path, &size);

  const GLint length = (GLint)size;
  glShaderSource(id, 1, &shader_source, &length);

  glCompileShader(id);

  GLint status = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    GLint info_log_length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);

    char *info_log = malloc(info_log_length * sizeof(char));
    glGetShaderInfoLog(id, info_log_length, &info_log_length, info_log);

    fputs(info_log, stderr);

    free(info_log);

    window_fail_with_error("Error compiling shader!");
  }
  return id;
}

void assets_gl_check_program(GLuint program) {
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    window_fail_with_error("Error linking program!");
  }
}

GLuint assets_gl_create_texture(const char *file_path) {
  GLuint id = 0;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  size_t data_size;
  char *data = NULL;
  data = assets_read_file(file_path, &data_size);

  int width = 0, height = 0;
  int num_channels = 0;
  stbi_uc *pixels =
      stbi_load_from_memory((const stbi_uc *)data, data_size, &width, &height,
                            &num_channels, STBI_rgb_alpha);
  if (pixels == NULL) {
    window_fail_with_error("Error loading image!");
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  stbi_image_free(pixels);

  return id;
}
