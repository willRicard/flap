#ifndef FLAP_ASSETS_H_
#define FLAP_ASSETS_H_

#include <stddef.h>

/**
 * Read a file
 */
char *assets_read_file(const char *file_path, size_t *data_size);

#endif // FLAP_ASSETS_H_
