#!/bin/bash
# CMake 快速构建脚本 (Linux/macOS)
# 使用方法：./build.sh

set -e

BUILD_TYPE="${1:-Debug}"
QT_PATH="${2:-}"

echo "====================================="
echo "LanChat CMake 构建脚本"
echo "====================================="
echo ""

# 清理旧的 build 目录
if [ -d "build" ]; then
    echo "清理旧的构建目录..."
    rm -rf build
fi

# 创建 build 目录
echo "创建构建目录..."
mkdir build
cd build

# 配置 CMake
echo "配置 CMake 项目..."
CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

if [ -n "$QT_PATH" ]; then
    echo "使用指定的 Qt 路径: $QT_PATH"
    CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=$QT_PATH")
elif command -v brew &> /dev/null; then
    # 在 macOS 上尝试使用 Homebrew 的 Qt
    if brew list qt@6 &> /dev/null; then
        QT_BREW_PATH=$(brew --prefix qt@6)
        echo "检测到 Homebrew Qt6: $QT_BREW_PATH"
        CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=$QT_BREW_PATH")
    elif brew list qt@5 &> /dev/null; then
        QT_BREW_PATH=$(brew --prefix qt@5)
        echo "检测到 Homebrew Qt5: $QT_BREW_PATH"
        CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=$QT_BREW_PATH")
    fi
fi

cmake .. "${CMAKE_ARGS[@]}"

echo ""
echo "====================================="
echo "CMake 配置成功！"
echo "====================================="
echo ""
echo "下一步操作："
echo "  1. 编译项目: cmake --build . -j\$(nproc)"
echo "  2. 或者直接: make -j\$(nproc)"
echo ""

# 询问是否立即编译
read -p "是否立即编译项目？(Y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
    echo ""
    echo "开始编译..."
    
    # 获取 CPU 核心数
    if [[ "$OSTYPE" == "darwin"* ]]; then
        NPROC=$(sysctl -n hw.ncpu)
    else
        NPROC=$(nproc)
    fi
    
    cmake --build . -j$NPROC
    
    echo ""
    echo "====================================="
    echo "编译成功！"
    echo "====================================="
    echo ""
    echo "可执行文件位置: ./bin/LanChat"
fi

cd ..
