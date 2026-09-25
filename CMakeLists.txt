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
#  着色器:源码收集 + 随构建自动编译(glslc)
#    源码目录:Vulcanite/assets/shader
#    输出目录:Vulcanite/assets/spir_v
#    命名映射:*.vert -> vert.spv   *.frag -> frag.spv   其他 -> <文件名>.spv
#  源码未变化时 glslc 不会重复执行(由 DEPENDS 驱动增量)
# ============================================================================
set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/Vulcanite/assets/shader)
set(SHADER_OUT_DIR ${CMAKE_SOURCE_DIR}/Vulcanite/assets/spir_v)

file(GLOB_RECURSE VULCANITE_SHADERS CONFIGURE_DEPENDS
    ${SHADER_SRC_DIR}/*.vert
    ${SHADER_SRC_DIR}/*.frag
    ${SHADER_SRC_DIR}/*.comp
    ${SHADER_SRC_DIR}/*.glsl
)

# 查找 glslc:优先用 FindVulkan 给出的路径,否则从 VULKAN_SDK/Bin 找
if(Vulkan_GLSLC_EXECUTABLE)
    set(GLSLC_EXECUTABLE ${Vulkan_GLSLC_EXECUTABLE})
else()
    find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin")
endif()
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "未找到 glslc!请安装 Vulkan SDK 并确保 VULKAN_SDK 环境变量有效(可运行: python setup_vulkan.py --auto)")
endif()
message(STATUS "glslc: ${GLSLC_EXECUTABLE}")

file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SHADER_SPV_OUTPUTS "")
foreach(SHADER_SRC ${VULCANITE_SHADERS})
    get_filename_component(SHADER_EXT  ${SHADER_SRC} EXT)
    get_filename_component(SHADER_NAME ${SHADER_SRC} NAME_WE)

    if(SHADER_EXT STREQUAL ".vert")
        set(SHADER_SPV ${SHADER_OUT_DIR}/vert.spv)
    elseif(SHADER_EXT STREQUAL ".frag")
        set(SHADER_SPV ${SHADER_OUT_DIR}/frag.spv)
    else()
        set(SHADER_SPV ${SHADER_OUT_DIR}/${SHADER_NAME}.spv)
    endif()

    add_custom_command(
        OUTPUT  ${SHADER_SPV}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SRC} -o ${SHADER_SPV}
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling shader: ${SHADER_NAME}${SHADER_EXT}"
        VERBATIM
    )
    list(APPEND SHADER_SPV_OUTPUTS ${SHADER_SPV})
endforeach()

# ALL:每次构建都参与检查(DEPENDS 未变化则跳过实际编译)
if(SHADER_SPV_OUTPUTS)
    add_custom_target(CompileShaders ALL DEPENDS ${SHADER_SPV_OUTPUTS})
endif()

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

# 着色器源码:归入 VS 的 "Shaders" 分组;HEADER_FILE_ONLY 避免 MSVC 尝试编译 GLSL
if(VULCANITE_SHADERS)
    set_source_files_properties(${VULCANITE_SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
    source_group(TREE ${SHADER_SRC_DIR} PREFIX "Shaders" FILES ${VULCANITE_SHADERS})
endif()

add_library(Vulcanite STATIC
    ${VULCANITE_CPP}
    ${VULCANITE_HPP}
    ${VULCANITE_SHADERS}
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

# 确保 spv 编译完成后,再执行下面的资产复制
if(TARGET CompileShaders)
    add_dependencies(SandBox CompileShaders)
endif()

# ===== 构建后把 spir_v(着色器编译产物)复制到 exe 旁边 =====
add_custom_command(TARGET SandBox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADER_OUT_DIR}
    $<TARGET_FILE_DIR:SandBox>/assets/spir_v
    COMMENT "Copying spir_v to SandBox output directory"
)

# ===== VS 调试时工作目录设为 exe 所在目录(相对路径 assets/... 直接命中) =====
set_target_properties(SandBox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:SandBox>
)

# ===== 让 VS 默认以 SandBox 作为启动项目(否则会尝试运行 Vulcanite.lib 而报"不是 Win32 程序") =====
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SandBox)
