cmake_minimum_required(VERSION 3.20)

# ===== 项目 =====
project(Vulcanite VERSION 0.1.0 LANGUAGES CXX)

# ===== 全局编译选项 =====
if(MSVC)
    add_compile_options(/W3 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Debug 构建定义 VULCANITE_DEBUG,启用日志输出
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    add_compile_definitions(VULCANITE_DEBUG)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 输出目录统一到 bin/ (替代原 bin/bin_int) =====
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ===== 第三方库:spdlog (子模块,编译模式) =====
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(Vendor/spdlog)

# ===== 自动收集引擎源码 (src 下所有 .cpp 和 .h) =====
file(GLOB_RECURSE VULCANITE_CPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Vulcanite/src/*.cpp
)
file(GLOB_RECURSE VULCANITE_HPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Vulcanite/src/*.h
)

# ===== 按目录生成 VS 虚拟文件夹 (core 子目录) =====
source_group(TREE ${CMAKE_SOURCE_DIR}/Vulcanite/src PREFIX "Source Files" FILES ${VULCANITE_CPP})
source_group(TREE ${CMAKE_SOURCE_DIR}/Vulcanite/src PREFIX "Header Files" FILES ${VULCANITE_HPP})

# ===== 主程序 =====
add_executable(Vulcanite
    ${VULCANITE_CPP}
    ${VULCANITE_HPP}
)

target_link_libraries(Vulcanite PRIVATE spdlog::spdlog)
target_include_directories(Vulcanite PRIVATE
    ${CMAKE_SOURCE_DIR}/Vulcanite/src
)
