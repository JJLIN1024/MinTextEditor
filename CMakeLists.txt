cmake_minimum_required(VERSION 3.0)

project(MinTextEditor VERSION 0.1)

set(SOURCES src/main.c src/editor.c src/editor.h)
add_executable(minTextEditor ${SOURCES})
