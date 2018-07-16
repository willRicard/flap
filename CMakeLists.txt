cmake_minimum_required(VERSION 3.11)

project(flap)

find_package(glfw3 REQUIRED)

find_package(Vulkan)
find_package(OpenGL)
find_package(GLEW)

if(Vulkan_FOUND AND NOT USE_OPENGL)
  add_executable(flap
    main.c
    rect.c
    vk/init.c
    vk/window.c
    vk/renderer.c
    vk/pipeline.c
    vk/rect.c)

  target_include_directories(flap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(flap PUBLIC
    Vulkan::Vulkan
    glfw)

elseif(OpenGL_FOUND)

  add_executable(flap
    main.c
    rect.c
    gl/init.c
    gl/window.c
    gl/shader.c
    gl/rect.c)

  target_include_directories(flap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(flap PUBLIC
    OpenGL::GL
    GLEW::GLEW
    glfw)

endif()

