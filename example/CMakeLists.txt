CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(echo)

SET(CMAKE_BUILD_TYPE "debug")
SET(C_FLAGS
    -g                  # for gdb
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-switch-enum
    -rdynamic           # for backtrace
    -Wconversion
    -Wpointer-arith
    -march=native
    -Werror
)

STRING(REPLACE ";" " " CMAKE_C_FLAGS "${C_FLAGS}")
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_C_FLAGS_DEBUG "-o0")     
SET(CMAKE_C_FLAGS_RELEASE "-o2 -finline-limit=1000 -DNDEBUG")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../lib)
FILE(GLOB 37NET "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.c")
FILE(GLOB 37LOG "${CMAKE_CURRENT_SOURCE_DIR}/../lib/*.c")
ADD_EXECUTABLE(echo echo.c ${37LOG} ${37NET})
