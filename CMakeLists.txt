cmake_minimum_required(VERSION 3.18)
project(texteditor)

set(OpenGL_GL_PREFERENCE GLVND) # for freetype-gl

add_subdirectory(freetype-gl)

include_directories(
    freetype-gl
    freetype-gl/demos
)

set(CMAKE_BUILD_TYPE Debug)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    freetype-gl/demos/mat4.c
    freetype-gl/demos/shader.c
)

target_link_libraries(
    ${PROJECT_NAME}
    freetype-gl
    glfw
    m
    GLU
    GLEW
    GL
    freetype
)