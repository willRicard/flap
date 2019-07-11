cmake_minimum_required(VERSION 3.10)

project(flap)

add_subdirectory(sulfur)

if(FLAP_ANDROID)
        find_library(ANDROID_LIBRARY android)
        find_library(ANDROID_LOG_LIBRARY log)
        find_library(VULKAN_LIBRARY vulkan)

        add_library(flap SHARED
          ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
          src/main.c
          src/assets.h
          src/assets.c
          src/assets_android.c
          src/window.h
          src/window_android.c
          src/game.h
          src/game.c
          src/sprite.h
          src/sprite.c)

        target_include_directories(flap PUBLIC
          ${ANDROID_NDK}/sources/android/native_app_glue)

        target_link_libraries(flap PUBLIC
          Sulfur::Sulfur
          ${ANDROID_LOG_LIBRARY}
          ${ANDROID_LIBRARY}
          ${VULKAN_LIBRARY})
else(FLAP_ANDROID)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(flap
  src/main.c
  src/assets.h
  src/assets.c
  src/assets_desktop.c
  src/window.h
  src/window_desktop.c
  src/game.h
  src/game.c
  src/sprite.h
  src/sprite.c)

target_link_libraries(flap PUBLIC
  Sulfur::Sulfur
  Vulkan::Vulkan
  glfw)

endif(FLAP_ANDROID)
