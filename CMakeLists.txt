cmake_minimum_required(VERSION 3.16)

project(Coral2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ==============================
# SDL3
# ==============================

add_subdirectory(
    3rd_party/SDL
    EXCLUDE_FROM_ALL
)


# ==============================
# LuaJIT
# ==============================

set(
    LUAJIT_DIR
    "${CMAKE_SOURCE_DIR}/3rd_party/LuaJIT"
)

add_library(
    LuaJIT::LuaJIT
    SHARED
    IMPORTED
)

set_target_properties(
    LuaJIT::LuaJIT
    PROPERTIES

    IMPORTED_IMPLIB
        "${LUAJIT_DIR}/src/lua51.lib"

    IMPORTED_LOCATION
        "${LUAJIT_DIR}/src/lua51.dll"

    INTERFACE_INCLUDE_DIRECTORIES
        "${LUAJIT_DIR}/src"
)


# ==============================
# 自动收集 src 下所有 cpp
# ==============================

file(
    GLOB_RECURSE
    PROJECT_SOURCES

    CONFIGURE_DEPENDS

    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)


# ==============================
# 主程序
# ==============================

add_executable(
    ${PROJECT_NAME}

    ${PROJECT_SOURCES}
)


# src 作为 include 根目录
#
# 因此可以：
# #include "core/runtime/Frame/Frame.h"

target_include_directories(
    ${PROJECT_NAME}

    PRIVATE

    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)


# ==============================
# 链接
# ==============================

target_link_libraries(
    ${PROJECT_NAME}

    PRIVATE

    SDL3::SDL3
    LuaJIT::LuaJIT
)


# ==============================
# 复制 LuaJIT DLL
# ==============================

add_custom_command(
    TARGET ${PROJECT_NAME}

    POST_BUILD

    COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different

        "${LUAJIT_DIR}/src/lua51.dll"

        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

add_custom_target(
    copy_scripts
    ALL

    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts"
        "${CMAKE_CURRENT_BINARY_DIR}/scripts"

    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/shaders"

    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/doc"
        "${CMAKE_CURRENT_BINARY_DIR}/doc"

    COMMENT
        "Copying Lua scripts and docs"
)

add_dependencies(
    ${PROJECT_NAME}
    copy_scripts
)