#include "assets.h"
#include "window_android.h"

#include <stdlib.h>
#include <string.h>

char *assets_read_file(const char *file_path, size_t *data_size) {
  struct android_app *app = android_window_get_app();

  // Try reading from:
  // 1. Assets
  // 2. Internal data directory
  // 3. External data directory

  AAssetManager *asset_manager = app->activity->assetManager;

  AAsset *asset = NULL;
  asset = AAssetManager_open(asset_manager, file_path, AASSET_MODE_BUFFER);

  if (asset != NULL) {
    size_t size = (size_t)AAsset_getLength(asset);

    char *data = (char *)malloc(size * sizeof(char));

    AAsset_read(asset, data, size);

    AAsset_close(asset);

    *data_size = size;
    return data;
  }

  const size_t max_len = 64;
  char full_path[max_len] = {0};

  size_t dir_len = strlen(app->activity->internalDataPath);

  strncat(full_path, app->activity->internalDataPath, max_len);
  strncat(full_path, file_path, max_len - dir_len);

  char *data = assets_base_read_file(full_path, data_size);
  if (data != NULL) {
    return data;
  }

  memset(full_path, 0, sizeof(full_path));

  dir_len = strlen(app->activity->externalDataPath);
  strncat(full_path, app->activity->externalDataPath, max_len);
  strncat(full_path, file_path, max_len - dir_len);

  data = assets_base_read_file(full_path, data_size);
  if (data != NULL) {
    return data;
  }

  return NULL;
}

void assets_write_file(const char *data, size_t data_size,
                       const char *file_path) {
  struct android_app *app = android_window_get_app();

  const size_t max_len = 64;

  char full_path[max_len] = {0};

  size_t dir_len = strlen(app->activity->internalDataPath);

  strncat(full_path, app->activity->internalDataPath, max_len);
  strncat(full_path, file_path, max_len - dir_len);

  assets_base_write_file(data, data_size, full_path);
}
