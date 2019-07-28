#include "assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"

/**
 * Read contents of `file_path` using standard C calls.
 * You are responsible for freeing the allocated memory.
 */
char *assets_base_read_file(const char *file_path, size_t *data_size) {
  char full_path[64] = {'a', 's', 's', 'e', 't', 's', '/'};

#ifdef _WIN32
  strncat_s(full_path, 64, file_path, 56);
#else
  strncat(full_path, file_path, 56);
#endif

  FILE *file = NULL;

#ifdef _WIN32
  fopen_s(&file, full_path, "rb");
#else
  file = fopen(full_path, "rbe");
#endif

  if (file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  const size_t size = (size_t)ftell(file);

  char *data = NULL;
  data = (char *)malloc(size * sizeof(char));
  if (data == NULL) {
    return NULL;
  }

  rewind(file);
  fread(data, size, 1, file);

  fclose(file);

  if (data_size != NULL) {
    *data_size = size;
  }

  return data;
}

/**
 * Write `data` to `file_path` using standard C calls.
 */
void assets_base_write_file(const char *data, size_t data_size,
                            const char *file_path) {
#ifdef _WIN32
  FILE *file = fopen(file_path, "wb");
#else
  FILE *file = fopen(file_path, "wbe");
#endif

  fwrite(data, data_size, 1, file);

  fclose(file);
}
