#include "assets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *assets_read_file(const char *file_path, size_t *data_size) {
  return assets_base_read_file(file_path, data_size);
}

void assets_write_file(const char *data, size_t data_size,
                       const char *file_path) {
  assets_base_write_file(data, data_size, file_path);
}
