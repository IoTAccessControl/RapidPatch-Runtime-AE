cmake_minimum_required(VERSION 3.4)

project(LibEbpf VERSION 0.1 LANGUAGES C CXX)

set(APP_TARGET LibEbpf)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_definitions(-DWin32)

SET(CMAKE_C_FLAGS "-g")

file(GLOB libsrc
        "${PROJECT_SOURCE_DIR}/include/*.h"
		"${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.c")

include_directories(
    "${PROJECT_SOURCE_DIR}/"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
)

add_definitions(-DLINUX_TEST)

add_executable(ebpf
    "${libsrc}"
    "${PROJECT_SOURCE_DIR}/src/main.c"
)

set_target_properties(ebpf PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    OUTPUT_NAME "ebpf.elf"
    )