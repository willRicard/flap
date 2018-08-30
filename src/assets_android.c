#include "assets.h"
#include "window_android.h"

#include <stdlib.h>
#include <stdio.h>

char *flapAssetsReadFile(const char *filePath, size_t *dataSize) {
  struct android_app *app = flapGetApp();
  AAssetManager *assetManager = app->activity->assetManager;

  AAsset *asset = NULL;
  asset = AAssetManager_open(assetManager, filePath, AASSET_MODE_BUFFER);

  if (asset == NULL) {
    return NULL;
  }

  off_t size = AAsset_getLength(asset);

  char *data = (char*) malloc(size * sizeof(char));

  AAsset_read(asset, data, size);

  AAsset_close(asset);

  *dataSize = size;
  return data;
}
