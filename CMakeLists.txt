cmake_minimum_required(VERSION 3.10)

project(flap)

if(ANDROID)
  add_subdirectory(sulfur)

  add_library(
    flap SHARED
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    src/main_vk.c
    src/assets.c
    src/assets_android.c
    src/assets_vk.c
    src/window_android.c
    src/game.c
    src/sprite_vk.c)

  target_include_directories(
    flap
    PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

  target_link_libraries(flap
                        PUBLIC Sulfur::Sulfur
                               android
                               log
                               vulkan)
else()

  if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    find_package(glfw3 REQUIRED)
  else()
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
  endif()

  find_package(Vulkan)

  if(Vulkan_FOUND AND NOT USE_OPENGL)
    add_subdirectory(sulfur)
    add_executable(flap
                   src/main_vk.c
                   src/assets.c
                   src/assets_desktop.c
                   src/assets_vk.c
                   src/window_desktop.c
                   src/window_desktop_vk.c
                   src/game.c
                   src/sprite_vk.c)

    target_link_libraries(flap PUBLIC Sulfur::Sulfur Vulkan::Vulkan glfw)
  else()
    set(OpenGL_GL_PREFERENCE "GLVND")
    find_package(OpenGL REQUIRED)
    add_executable(flap
                   glad/src/glad.c
                   src/main_gl.c
                   src/assets.c
                   src/assets_desktop.c
                   src/assets_gl.c
                   src/window_desktop.c
                   src/window_desktop_gl.c
                   src/game.c
                   src/sprite_gl.c)
    target_include_directories(flap PUBLIC glad/include)

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(flap
                              PUBLIC "-s ASSERTIONS=2" "-s NO_WASM"
                                     "-s GL_DEBUG"
                                     "-s GL_ASSERTIONS"
                                     "--emrun"
                                     "--preload-file ../assets"
                                     "-s USE_GLFW=3"
                                     GL
                                     glfw)

      else()
        target_link_libraries(flap
                              PUBLIC "-s NO_ASSERTIONS"
                                     "--preload-file ../assets"
                                     "-s USE_GLFW=3"
                                     GL
                                     glfw)
      endif()
    else()
      target_link_libraries(flap PUBLIC OpenGL::GL glfw)
    endif()

  endif()

  if(NOT WIN32)
    target_link_libraries(flap PUBLIC m dl)
  endif()

endif()
