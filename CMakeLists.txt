cmake_minimum_required(VERSION 3.12)

project(flap)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(flap
  src/main.c
  src/rect.c
  src/assets_desktop.c
  src/window_desktop.c
  src/window_desktop.c
  src/renderer.c
  src/pipeline.c
  src/rect.c)

target_include_directories(flap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(flap PUBLIC
  Vulkan::Vulkan
  glfw)
