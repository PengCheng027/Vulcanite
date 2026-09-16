cmake_minimum_required(VERSION 3.20)

# ===== 项目 =====
project(Vulcanite VERSION 0.1.0 LANGUAGES CXX)

# ===== 全局编译选项 =====
if(MSVC)
    add_compile_options(/W3 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Debug 构建定义 VULCANITE_DEBUG(启用日志)与 VULCANITE_ENABLE_ASSERTS(启用断言)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    add_compile_definitions(VULCANITE_DEBUG)
    add_compile_definitions(VULCANITE_ENABLE_ASSERTS)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 输出目录统一到 bin/ =====
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ===== 第三方库:spdlog (子模块,编译模式) =====
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(Vendor/spdlog)

# ===== 第三方库:GLFW (子模块,静态库) =====
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(Vendor/glfw)

# ===== Vulkan SDK (需 VULKAN_SDK 环境变量,缺失时用 setup_vulkan.py 安装) =====
if(NOT DEFINED ENV{VULKAN_SDK})
    message(WARNING "未检测到 VULKAN_SDK 环境变量!请运行: python setup_vulkan.py --auto")
endif()
find_package(Vulkan REQUIRED)

# ============================================================================
#  引擎静态库 (Vulcanite)
#  收集 Vulcanite/src 下所有源码,排除入口 main.cpp(入口属于应用层)
# ============================================================================
file(GLOB_RECURSE VULCANITE_CPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Vulcanite/src/*.cpp
)
file(GLOB_RECURSE VULCANITE_HPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Vulcanite/src/*.h
)

# 引擎是库,不含 main 入口
list(FILTER VULCANITE_CPP EXCLUDE REGEX "main\\.cpp$")

source_group(TREE ${CMAKE_SOURCE_DIR}/Vulcanite/src PREFIX "Source Files" FILES ${VULCANITE_CPP})
source_group(TREE ${CMAKE_SOURCE_DIR}/Vulcanite/src PREFIX "Header Files" FILES ${VULCANITE_HPP})

add_library(Vulcanite STATIC
    ${VULCANITE_CPP}
    ${VULCANITE_HPP}
)

# PUBLIC:让链接引擎的目标(SandBox)自动继承依赖与 include 路径
target_link_libraries(Vulcanite PUBLIC spdlog::spdlog glfw Vulkan::Vulkan)
target_include_directories(Vulcanite PUBLIC
    ${CMAKE_SOURCE_DIR}/Vulcanite/src
    ${CMAKE_SOURCE_DIR}/Vendor/glm
)

# ============================================================================
#  示例应用 (SandBox) —— 引擎的使用者
# ============================================================================
file(GLOB_RECURSE SANDBOX_CPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/SandBox/src/*.cpp
)
file(GLOB_RECURSE SANDBOX_HPP CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/SandBox/src/*.h
)

source_group(TREE ${CMAKE_SOURCE_DIR}/SandBox/src PREFIX "Source Files" FILES ${SANDBOX_CPP})
source_group(TREE ${CMAKE_SOURCE_DIR}/SandBox/src PREFIX "Header Files" FILES ${SANDBOX_HPP})

add_executable(SandBox
    ${SANDBOX_CPP}
    ${SANDBOX_HPP}
)

target_link_libraries(SandBox PRIVATE Vulcanite)

# ===== 构建后把 spir_v 复制到 exe 旁边(任意工作目录都能读已编译 shader) =====
add_custom_command(TARGET SandBox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Vulcanite/assets/spir_v
    $<TARGET_FILE_DIR:SandBox>/assets/spir_v
    COMMENT "Copying spir_v to SandBox output directory"
)

# ===== VS 调试时工作目录设为 exe 所在目录(相对路径 assets/... 直接命中) =====
set_target_properties(SandBox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:SandBox>
)

# ===== 让 VS 默认以 SandBox 作为启动项目(否则会尝试运行 Vulcanite.lib 而报"不是 Win32 程序") =====
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SandBox)
