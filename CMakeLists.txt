cmake_minimum_required(VERSION 3.11)

project(flap)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_library(GLFW_LIBRARY NAMES glfw glfw3)

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)

add_executable(flap
  main.c
  window.c
  shader.c
  rect.c)

target_include_directories(flap PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR})

target_link_libraries(flap PUBLIC
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARY} 
  ${GLFW_LIBRARY})
