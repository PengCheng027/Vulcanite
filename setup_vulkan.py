#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Vulcanite - Vulkan SDK 检测与安装脚本

用法:
    python setup_vulkan.py          # 检测 SDK,缺失则提示
    python setup_vulkan.py --auto   # 检测 SDK,缺失则自动下载安装
    python setup_vulkan.py --path C:/MySDK   # 手动指定 SDK 路径
    python setup_vulkan.py --check  # 只检测,不安装

说明:
    优先使用 VULKAN_SDK 环境变量;未设置时探测常见安装路径;
    都找不到时,按 --auto 自动下载 LunarG 官方 SDK 安装器并静默安装。
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import urllib.request
from pathlib import Path

# LunarG SDK 最新稳定版本(如版本更新可修改)
SDK_VERSION = "1.3.290.0"
SDK_BASE_URL = "https://sdk.lunarg.com/sdk/download"

# 需要验证的关键文件(相对 SDK 根目录)
REQUIRED_FILES = [
    "Bin/glslc.exe",
    "Include/vulkan/vulkan.h",
]

# 常见探测路径(Windows)
PROBE_PATHS = [
    "C:/VulkanSDK",
    "%LOCALAPPDATA%/VulkanSDK",
    "%PROGRAMFILES%/VulkanSDK",
    "%PROGRAMFILES(X86)%/VulkanSDK",
]


def log(msg: str, level: str = "INFO"):
    colors = {"INFO": "\033[36m", "OK": "\033[32m", "WARN": "\033[33m", "ERR": "\033[31m"}
    reset = "\033[0m"
    prefix = colors.get(level, colors["INFO"])
    print(f"{prefix}[{level}]{reset} {msg}")


def expand_env(path: str) -> str:
    """展开路径中的 %VAR% 环境变量"""
    return os.path.expandvars(os.path.expanduser(path))


def find_sdk_in_paths() -> list:
    """在常见路径下探测存在的 SDK 目录"""
    found = []
    for raw in PROBE_PATHS:
        base = expand_env(raw)
        if not os.path.isdir(base):
            continue
        for entry in os.listdir(base):
            candidate = os.path.join(base, entry)
            if os.path.isdir(candidate) and os.path.isfile(os.path.join(candidate, "Include/vulkan/vulkan.h")):
                found.append(candidate)
    return found


def validate_sdk(path: str) -> bool:
    """验证 SDK 路径是否包含关键文件"""
    for rel in REQUIRED_FILES:
        if not os.path.isfile(os.path.join(path, rel)):
            log(f"SDK 缺少文件: {rel}", "ERR")
            return False
    return True


def detect_sdk() -> str | None:
    """检测已安装的 Vulkan SDK,返回路径或 None"""
    # 1. 环境变量
    env = os.environ.get("VULKAN_SDK")
    if env and validate_sdk(env):
        log(f"通过 VULKAN_SDK 环境变量找到: {env}", "OK")
        return env
    # 2. 常见路径探测
    for path in find_sdk_in_paths():
        if validate_sdk(path):
            log(f"在探测路径找到 SDK: {path}", "OK")
            return path
    return None


def download_sdk_installer() -> str:
    """下载 LunarG SDK 安装器到临时目录,返回安装器路径"""
    system = platform.system()
    arch = platform.architecture()[0]  # 64bit / 32bit
    if system != "Windows":
        log(f"当前平台 {system} 暂不支持自动安装,请手动安装 Vulkan SDK", "ERR")
        sys.exit(1)

    ext = "exe" if arch == "64bit" else "exe"  # LunarG Windows 安装器统一为 exe
    url = f"{SDK_BASE_URL}/{SDK_VERSION}/windows/VulkanSDK-{SDK_VERSION}-Installer.exe"
    # 官方下载重定向到最终文件,需要跟随重定向
    log(f"下载 Vulkan SDK {SDK_VERSION} 安装器...")
    log(f"URL: {url}")
    tmp = Path(tempfile.gettempdir()) / f"VulkanSDK-{SDK_VERSION}-Installer.exe"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
        with urllib.request.urlopen(req, timeout=60) as resp, open(tmp, "wb") as f:
            shutil.copyfileobj(resp, f)
        log(f"下载完成: {tmp}", "OK")
        return str(tmp)
    except Exception as e:
        log(f"下载失败: {e}", "ERR")
        log("请手动到 https://vulkan.lunarg.com/sdk/home 下载安装", "WARN")
        sys.exit(1)


def install_sdk(installer_path: str, target: str) -> None:
    """静默安装 SDK 到指定目录"""
    os.makedirs(target, exist_ok=True)
    log(f"静默安装到: {target}")
    # LunarG 安装器支持 /S 静默安装
    cmd = [installer_path, "/S", f"/dir={target}"]
    try:
        result = subprocess.run(cmd, capture_output=True, timeout=600)
        if result.returncode != 0:
            log(f"安装器返回非零码: {result.returncode}", "WARN")
    except subprocess.TimeoutExpired:
        log("安装超时,请手动检查安装器", "WARN")
    except Exception as e:
        log(f"启动安装器失败: {e}", "ERR")
        sys.exit(1)


def set_env_var(name: str, value: str) -> None:
    """写入用户级环境变量(Windows)"""
    if platform.system() == "Windows":
        try:
            subprocess.run(["setx", name, value], check=True, capture_output=True)
            log(f"已写入用户环境变量 {name}={value}", "OK")
            log("注意: 请重新打开终端/VS 使环境变量生效", "WARN")
        except Exception as e:
            log(f"写入环境变量失败: {e}(请手动设置 {name}={value})", "WARN")


def main():
    parser = argparse.ArgumentParser(description="Vulcanite - Vulkan SDK setup")
    parser.add_argument("--auto", action="store_true", help="缺失时自动下载安装")
    parser.add_argument("--path", type=str, help="手动指定 SDK 路径")
    parser.add_argument("--check", action="store_true", help="只检测不安装")
    args = parser.parse_args()

    log("=== Vulkan SDK 检测 ===")

    # 手动指定路径
    if args.path:
        p = expand_env(args.path)
        if validate_sdk(p):
            log(f"手动路径有效: {p}", "OK")
            set_env_var("VULKAN_SDK", p)
            return
        log(f"手动路径无效: {p}", "ERR")
        sys.exit(1)

    # 检测已有 SDK
    sdk = detect_sdk()
    if sdk:
        log(f"已找到 Vulkan SDK: {sdk}", "OK")
        if not os.environ.get("VULKAN_SDK"):
            set_env_var("VULKAN_SDK", sdk)
        return

    # 未找到
    log("未检测到 Vulkan SDK", "WARN")
    if args.check:
        log("仅检测模式,退出(未安装)", "WARN")
        sys.exit(1)

    if not args.auto:
        log("运行 python setup_vulkan.py --auto 可自动下载安装", "INFO")
        log("或手动下载: https://vulkan.lunarg.com/sdk/home", "INFO")
        sys.exit(1)

    # 自动下载安装
    log("开始自动安装流程...", "INFO")
    installer = download_sdk_installer()
    target = str(Path(os.environ.get("LOCALAPPDATA", os.path.expanduser("~"))) / "VulkanSDK" / SDK_VERSION)
    install_sdk(installer, target)
    if validate_sdk(target):
        log(f"安装成功: {target}", "OK")
        set_env_var("VULKAN_SDK", target)
    else:
        log("安装后验证失败,请手动检查", "ERR")
        sys.exit(1)


if __name__ == "__main__":
    main()
