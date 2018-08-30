#include <stdio.h>
#include <string.h>

char *flapAssetsReadFile(const char *filePath, size_t *dataSize) {
  char fullPath[32] = {0};
#ifdef _WIN32
  strncat_s(fullPath, 32, "assets/", 32);
  strncat_s(fullPath, 32, filePath, 32);
#else
  strncat(fullPath, "assets/", 32);
  strncat(fullPath, filePath, 32);
#endif

  FILE *file = NULL;
#ifdef _WIN32
  fopen_s(&file, fullPath, "rb");
#else
  file = fopen(fullPath, "rb");
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
  *dataSize = size;
  return data;
}
