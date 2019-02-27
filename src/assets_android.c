#include "assets.h"
#include "window_android.h"

#include <stdlib.h>

char *assets_read_file(const char *file_path, size_t *data_size) {
  struct android_app *app = android_window_get_app();
  AAssetManager *asset_manager = app->activity->assetManager;

  AAsset *asset = NULL;
  asset = AAssetManager_open(asset_manager, file_path, AASSET_MODE_BUFFER);

  if (asset == NULL) {
    return NULL;
  }

  off_t size = AAsset_getLength(asset);

  char *data = (char *)malloc(size * sizeof(char));

  AAsset_read(asset, data, size);

  AAsset_close(asset);

  *data_size = size;
  return data;
}
