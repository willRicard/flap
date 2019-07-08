cmake_minimum_required(VERSION 3.12)

project(flap)

add_subdirectory(sulfur)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

find_path(STB_IMAGE_INCLUDE_DIR stb_image.h)

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

target_include_directories(flap PUBLIC ${STBI_IMAGE_INCLUDE_DIR})

target_link_libraries(flap PUBLIC
  Sulfur::Sulfur
  Vulkan::Vulkan
  glfw)
