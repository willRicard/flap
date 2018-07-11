cmake_minimum_required(VERSION 3.11)

project(flap)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(flap
  main.c
  window.c
  shader.c
  rect.c)

target_include_directories(flap PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR})

target_link_libraries(flap PUBLIC
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARY}
  glfw)
