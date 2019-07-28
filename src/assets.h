#ifndef FLAP_ASSETS_H
#define FLAP_ASSETS_H

#include <stddef.h>

char *assets_base_read_file(const char *file_path, size_t *data_size);

void assets_base_write_file(const char *data, size_t data_size,
                            const char *file_path);

char *assets_read_file(const char *file_path, size_t *data_size);

void assets_write_file(const char *data, size_t data_size,
                       const char *file_path);

#endif // FLAP_ASSETS_H
