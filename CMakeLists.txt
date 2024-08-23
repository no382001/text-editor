cmake_minimum_required(VERSION 3.18)
project(texteditor)

set(CMAKE_BUILD_TYPE Debug)

include_directories(h)

add_executable(
    ${PROJECT_NAME}
    src/bufferpool.c
    src/node.c
    src/linenode.c
    src/document.c
    src/editor.c
)