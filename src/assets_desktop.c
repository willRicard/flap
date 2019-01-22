#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *assets_read_file(const char *file_path, size_t *data_size) {
  char full_path[32] = {0};
#ifdef _WIN32
  strncat_s(full_path, 32, "assets/", 31);
  strncat_s(full_path, 32, file_path, 31);
#else
  strncat(full_path, "assets/", 31);
  strncat(full_path, file_path, 31);
#endif

  FILE *file = NULL;
#ifdef _WIN32
  fopen_s(&file, full_path, "rb");
#else
  file = fopen(full_path, "rb");
#endif
  if (file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);

  char *data = NULL;
  data = (char *)malloc(size * sizeof(char));
  if (data == NULL) {
    return NULL;
  }

  rewind(file);
  fread(data, size, 1, file);

  fclose(file);
  *data_size = size;
  return data;
}
