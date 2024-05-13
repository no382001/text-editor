project(${PROJECT_NAME})

cmake_minimum_required(VERSION 3.18)
set(OpenGL_GL_PREFERENCE GLVND) # for freetype-gl

add_subdirectory(freetype-gl)

add_executable(${PROJECT_NAME} main.c)
