# Vulcanite

基于 **Vulkan + GLFW + ImGui** 的图形引擎(开发中),使用 **CMake** 构建,依赖通过 **git 子模块**管理。

## 目录结构

```
Vulcanite/
├── CMakeLists.txt          # 构建配置(唯一需要维护的构建文件)
├── build.bat               # 一键配置 + 构建脚本
├── setup_vulkan.py         # Vulkan SDK 检测/安装脚本
├── Vulcanite/
│   └── src/
│       ├── main.cpp        # 程序入口
│       └── core/
│           ├── VulLog.h    # 日志门面(双日志器 + 宏)
│           └── VulLog.cpp
├── Vendor/                 # 第三方库(git 子模块)
│   ├── spdlog/             # 日志库(编译模式, v1.17.0)
│   ├── glfw/               # 窗口库(静态库, 3.5.1)
│   └── glm/                # 数学库(纯头文件, 1.0.3)
├── bin/                    # 编译输出(自动生成, 不入库)
└── build/                  # CMake 构建目录(自动生成, 不入库)
```

## 环境要求

- **Visual Studio 2022**(含 C++ 桌面开发工作负载,自带 CMake 与 MSBuild)
- **Git**(用于克隆仓库与子模块)
- **Python 3.x**(用于 Vulkan SDK 安装脚本)
- **Vulkan SDK**(见下方安装说明)
- 可选:独立安装的 [CMake](https://cmake.org/download/)(≥ 3.20;不装也能用 VS 自带的)

## 快速开始

### 1. 克隆仓库(含子模块)

```bash
git clone --recursive https://github.com/PengCheng027/Vulcanite.git
cd Vulcanite
```

> 如果忘记加 `--recursive`,子模块是空的,补拉:

```bash
git submodule update --init --recursive
```

### 2. 安装 Vulkan SDK(可选)

Vulkan 不会随 git 子模块自动安装。项目需要 `VULKAN_SDK` 环境变量指向已安装的 SDK,
使用仓库自带的脚本检测或安装:

```bash
python setup_vulkan.py            # 检测是否已安装(已装则直接显示路径)
python setup_vulkan.py --auto     # 未安装时自动下载安装官方 SDK(几百 MB)
python setup_vulkan.py --path C:/MySDK   # 手动指定 SDK 路径
```

> - 如果之前手动安装过 LunarG Vulkan SDK(默认会设置 `VULKAN_SDK` 环境变量),跳过此步;
> - `--auto` 会将 SDK 安装到 `%LOCALAPPDATA%\VulkanSDK\<版本>`,并写入用户环境变量;
> - **安装/设置环境变量后需重新打开终端或 VS** 使其生效。

### 3. 生成 VS 项目(两种方式任选)

**方式 A:使用 build.bat(推荐)**

```bat
build.bat              :: 自动 configure + 构建 Debug,生成 build/Vulcanite.sln
```

**方式 B:手动执行 CMake 命令**

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

生成完毕后,VS 项目位于 **`build/Vulcanite.sln`**,直接用 Visual Studio 打开即可。

### 4. 构建

```bat
build.bat              :: 构建 Debug
build.bat Release      :: 构建 Release
build.bat reconfigure  :: 强制重新配置(新增文件后建议执行)
build.bat clean        :: 删除 build 目录
```

或在 VS 中直接按 `Ctrl+Shift+B` 构建。

### 5. 运行

编译产物输出到 **`bin\<配置>\`**:

```
bin\Debug\Vulcanite.exe
```

运行后控制台会输出彩色日志(核心日志器 `Vulcanite` 与客户端日志器 `APP`),同时写入当前工作目录下 `log\Vulcanite.log`。

## 项目配置说明(CMakeLists.txt)

| 配置项 | 说明 |
|---|---|
| `stdcpp20` | C++20 标准 |
| `spdlog::spdlog` | 日志库,编译模式链接 |
| `glfw` | 窗口库,静态库链接(生成 `glfw3.lib`) |
| `Vulkan::Vulkan` | Vulkan SDK,通过 `find_package(Vulkan)` 链接 |
| GLM | 纯头文件数学库,include 路径已配置 |
| `VULCANITE_DEBUG` | Debug 构建自动定义,控制日志宏是否输出 |
| `VULCANITE_ENABLE_ASSERTS` | Debug 构建自动定义,启用断言 |
| `source_group` | 自动按目录生成 VS 虚拟文件夹(Header Files/core、Source Files/core) |
| 源码自动收集 | `src/` 下新增 `.cpp/.h` 无需改 CMake,reconfigure 后自动纳入 |

## 新增源文件后

1. 在 `Vulcanite/src/` 下按目录放好文件(如 `core/`、`vulkan/`、`renderer/`);
2. 重新配置:`build.bat reconfigure`(或 VS 里右键 `CMakeLists.txt` → 配置);
3. VS 解决方案资源管理器会自动按目录显示新文件。

## 常见问题

**Q: 打开 build/Vulcanite.sln 后找不到新加的头文件?**
重新 configure(`build.bat reconfigure`)——CMake 会把 `src/*.h` 重新收集进工程。

**Q: git clone 后 Vendor 里是空的?**
子模块未初始化,执行 `git submodule update --init --recursive`。

**Q: CMake 报错找不到 Vulkan?**
未设置 `VULKAN_SDK` 环境变量。运行 `python setup_vulkan.py --auto` 自动安装,然后**重新打开终端/VS**。

**Q: 构建报 spdlog 的 Unicode 相关错误?**
项目已配置 `/utf-8` 编译选项,若手动建工程需在编译选项中加入 `/utf-8`。
