cmake_minimum_required(VERSION 3.11)

project(IoTPatch VERSION 0.1 LANGUAGES C)

set(APP_TARGET IoTPatch)

file(GLOB libsrc
        "${PROJECT_SOURCE_DIR}/hotpatch/include/*.h"
        "${PROJECT_SOURCE_DIR}/hotpatch/src/*.c"
        "${PROJECT_SOURCE_DIR}/app/*.h"
        "${PROJECT_SOURCE_DIR}/app/*.c"
        "${PROJECT_SOURCE_DIR}/libebpf/include/*.h"
        "${PROJECT_SOURCE_DIR}/libebpf/src/*.c")

list(REMOVE_ITEM libsrc "${PROJECT_SOURCE_DIR}/libebpf/src/main.c")

include_directories(
        "${PROJECT_SOURCE_DIR}/"
        "${PROJECT_SOURCE_DIR}/hotpatch"
        "${PROJECT_SOURCE_DIR}/app"
        "${PROJECT_SOURCE_DIR}/libebpf"
)

add_definitions(-DLINUX_TEST)

add_executable(iotpatch
        "${libsrc}"
        "${PROJECT_SOURCE_DIR}/main.c")