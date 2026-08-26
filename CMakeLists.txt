cmake_minimum_required(VERSION 3.20)

# ===== 项目 =====
project(Vulcanite VERSION 0.1.0 LANGUAGES CXX)

# ===== 全局编译选项 =====
if(MSVC)
    add_compile_options(/W3 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
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

# ===== 主程序 =====
add_executable(Vulcanite
    Vulcanite/src/main.cpp
)

target_link_libraries(Vulcanite PRIVATE spdlog::spdlog)
target_include_directories(Vulcanite PRIVATE
    ${CMAKE_SOURCE_DIR}/Vulcanite/src
)
