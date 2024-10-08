cmake_minimum_required(VERSION 3.18)
project(texteditor)

set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    unity
    GIT_REPOSITORY "https://github.com/ThrowTheSwitch/Unity.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(unity)

set(CMAKE_BUILD_TYPE Debug)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

add_executable(
    ${PROJECT_NAME}
    src/bufferpool.c
    src/node.c
    src/linenode.c
    src/document.c
    src/utils.c
    src/editor.c
    src/cursor.c
    src/main.c
)
    
target_include_directories(${PROJECT_NAME} PRIVATE
    h
    ${raylib_SOURCE_DIR}/src
    ${unity_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} m raylib unity)

add_executable(
    ${PROJECT_NAME}_test
    src/bufferpool.c
    src/node.c
    src/linenode.c
    src/document.c
    src/utils.c
    src/tests.c
)
    
target_include_directories(${PROJECT_NAME}_test PRIVATE
    h
    ${raylib_SOURCE_DIR}/src
    ${unity_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}_test m unity)

