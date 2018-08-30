cmake_minimum_required(VERSION 3.11)

project(flap)

find_package(glfw3 REQUIRED)

find_package(Vulkan)
find_package(OpenGL)
find_package(GLEW)

if(Vulkan_FOUND AND NOT USE_OPENGL)
  add_executable(flap
    src/main.c
    src/rect.c
    src/assets_desktop.c
    src/window_desktop.c
    src/window_desktop_vk.c
    src/renderer_vk.c
    src/pipeline_vk.c
    src/rect_vk.c)

  target_include_directories(flap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(flap PUBLIC
    Vulkan::Vulkan
    glfw)

elseif(OpenGL_FOUND)

  add_executable(flap
    src/main.c
    src/rect.c
    src/window_desktop.c
    src/window_desktop_gl.c
    src/pipeline_gl.c
    src/rect_gl.c)

  target_include_directories(flap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(flap PUBLIC
    OpenGL::GL
    GLEW::GLEW
    glfw)

endif()

