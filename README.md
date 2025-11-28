# LanChat

<div align="center">

**一个基于 Qt 的轻量级局域网聊天应用**

</div>

---
## 📖 项目简介

LanChat 是一个基于 C++14 和 Qt 框架开发的跨平台局域网聊天应用。项目采用现代化的 CMake 构建系统，支持 Windows、Linux 和 macOS 三大平台。

### ✨ 主要特性

- 🚀 **跨平台支持**: Windows / Linux / macOS 一套代码，多平台运行
- 🎨 **现代化 UI**: 基于 Qt Widgets 构建的友好界面
- 🔧 **易于构建**: 使用 CMake 构建系统，支持多种 IDE
- 📦 **轻量级**: 无需复杂配置，开箱即用
- 🎯 **模块化设计**: 清晰的分层架构，易于扩展

### 🏗️ 系统架构

项目采用三层架构设计：

```
┌─────────────────────────────────────┐
│     表现层 (Presentation Layer)     │  → UI 组件、用户交互
├─────────────────────────────────────┤
│    业务逻辑层 (Service Layer)       │  → 核心业务逻辑
├─────────────────────────────────────┤
│   基础设施层 (Infrastructure)       │  → 网络、数据库、日志
└─────────────────────────────────────┘
```

详细架构设计请参考: [系统架构设计方案](doc/LanChat-Lite%20系统架构设计方案.md)
---

## 🚀 快速开始

### 前置要求

在开始之前，请确保你的系统已安装以下工具：

| 工具 | 版本要求 | 说明 |
|------|---------|------|
| **CMake** | ≥ 3.16 | 跨平台构建工具 |
| **Qt** | 6.x | GUI 框架 |
| **C++ 编译器** | 支持 C++14 | MSVC 2019+, GCC 5+, Clang 3.4+ |


## 🔨 构建项目

LanChat 提供了简便的自动化构建脚本，让你能够快速编译项目。

### Windows 构建

#### 方法 1: 使用自动构建脚本 ✨（推荐）

```powershell
# 在项目根目录打开 PowerShell

# 1. 如果 CMake 能自动找到 Qt，直接运行
.\build.ps1

# 2. 如果找不到 Qt，指定 Qt 安装路径
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"

# 3. 指定构建类型（Debug 或 Release）
.\build.ps1 -BuildType Release
```

**脚本会自动完成以下操作:**
- ✅ 清理旧的构建目录
- ✅ 配置 CMake 项目
- ✅ 询问是否立即编译
- ✅ 生成可执行文件到 `bin/` 目录

#### 方法 2: 手动构建

```powershell
# 创建构建目录
mkdir build
cd build

# 配置 CMake（指定 Qt 路径）
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2019_64"

# 编译
cmake --build . --config Debug

# 运行程序
cd ..
.\bin\LanChat.exe
```

---

### Linux 构建

#### 方法 1: 使用自动构建脚本 ✨（推荐）

```bash
# 在项目根目录打开终端

# 1. 赋予脚本执行权限（仅首次需要）
chmod +x build.sh

# 2. 运行构建脚本
./build.sh

# 3. 指定构建类型
./build.sh Release
```

#### 方法 2: 手动构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake ..

# 编译（使用多线程加速）
make -j$(nproc)

# 运行程序
cd ..
./bin/LanChat
```

---

### macOS 构建

#### 方法 1: 使用自动构建脚本 ✨（推荐）

```bash
# 在项目根目录打开终端

# 1. 赋予脚本执行权限（仅首次需要）
chmod +x build.sh

# 2. 运行构建脚本（脚本会自动检测 Homebrew 安装的 Qt）
./build.sh

# 3. 如果需要手动指定 Qt 路径
./build.sh Debug "/usr/local/opt/qt@6"
```

#### 方法 2: 手动构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake（使用 Homebrew 的 Qt）
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"

# 编译
make -j$(sysctl -n hw.ncpu)

# 运行程序
cd ..
./bin/LanChat
```

## 📁 项目结构

```
LanChat/
├── CMakeLists.txt          # CMake 配置文件
├── build.ps1               # Windows 自动构建脚本
├── build.sh                # Linux/macOS 自动构建脚本
├── README.md               # 本文档
├── .gitignore              # Git 忽略文件
│
├── bin/                    # 可执行文件输出目录（自动生成）
│   └── LanChat[.exe]       # 编译生成的程序
│
├── build/                  # CMake 构建目录（自动生成，不提交到 Git）
│
├── doc/                    # 项目文档
│   ├── LanChat-Lite 系统架构设计方案.md
│   ├── 核心开发工具(IDE & Framework).md
│   └── ...
│
├── src/                    # 源代码目录
│   ├── main.cpp            # 程序入口
│   ├── common/             # 公共定义（类型、常量等）
│   ├── ui/                 # UI 层
│   │   └── main_window/    # 主窗口
│   ├── service/            # 业务逻辑层
│   ├── model/              # 数据模型与 DAO
│   ├── network/            # 网络通信层
│   └── utils/              # 工具类（日志、配置、数据库）
│
└── LanChat.vcxproj         # Visual Studio 项目文件（可选，兼容保留）
```

---

## 📚 CMake 构建脚本使用说明

### `build.ps1` - Windows 构建脚本

**功能特性:**
- ✅ 自动清理旧的构建目录
- ✅ 智能检测 Qt 路径
- ✅ 支持自定义构建类型（Debug/Release）
- ✅ 支持自定义 Visual Studio 版本
- ✅ 交互式编译确认

**使用方法:**

```powershell
# 基础用法
.\build.ps1

# 指定 Qt 路径
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"

# 指定构建类型
.\build.ps1 -BuildType Release

# 指定 Visual Studio 版本
.\build.ps1 -Generator "Visual Studio 16 2019"

# 组合使用
.\build.ps1 -BuildType Release -QtPath "C:\Qt\6.5.3\msvc2019_64"
```

**参数说明:**

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `-BuildType` | 构建类型 | `Debug` | `Debug`, `Release` |
| `-Generator` | 生成器 | `Visual Studio 17 2022` | `Visual Studio 16 2019` |
| `-QtPath` | Qt 安装路径 | 自动检测 | `C:\Qt\6.5.3\msvc2019_64` |

---

### `build.sh` - Linux/macOS 构建脚本

**功能特性:**
- ✅ 自动清理旧的构建目录
- ✅ 智能检测 Homebrew 安装的 Qt（macOS）
- ✅ 支持自定义构建类型（Debug/Release）
- ✅ 自动使用多核编译加速
- ✅ 交互式编译确认

**使用方法:**

```bash
# 基础用法
./build.sh

# 指定构建类型
./build.sh Release

# 指定 Qt 路径
./build.sh Debug "/opt/Qt/6.5.3/gcc_64"

# macOS 使用 Homebrew Qt
./build.sh Debug "$(brew --prefix qt@6)"
```

**参数说明:**

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `$1` | 构建类型 | `Debug` | `Debug`, `Release` |
| `$2` | Qt 安装路径 | 自动检测 | `/opt/Qt/6.5.3/gcc_64` |

---

## ⚙️ CMake 配置说明

### 核心配置项

| 配置项 | 说明 | 值 |
|--------|------|-----|
| `CMAKE_CXX_STANDARD` | C++ 标准 | `14` |
| `CMAKE_AUTOMOC` | 自动 MOC | `ON` |
| `CMAKE_AUTOUIC` | 自动 UIC | `ON` |
| `CMAKE_AUTORCC` | 自动 RCC | `ON` |
| `CMAKE_RUNTIME_OUTPUT_DIRECTORY` | 可执行文件输出 | `${CMAKE_SOURCE_DIR}/bin` |

### 添加新的源文件

当你在项目中添加新的 `.cpp` 文件时，需要更新 `CMakeLists.txt`:

```cmake
# 在 SOURCES 变量中添加新文件
set(SOURCES
    src/main.cpp
    src/your_new_file.cpp  # 添加这里
)
```

然后重新运行构建脚本或执行 `cmake --build build`。

---

## 🐛 常见问题

<details>
<summary><b>Q: CMake 找不到 Qt？</b></summary>

**A:** 手动指定 Qt 路径：

```bash
# Windows
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"

# Linux/macOS
./build.sh Debug "/path/to/qt"
```

或者设置环境变量：
```bash
export CMAKE_PREFIX_PATH="/path/to/qt"
```
</details>

<details>
<summary><b>Q: 编译时出现 "Qt 版本不匹配" 错误？</b></summary>

**A:** 确保你的编译器与 Qt 版本匹配：
- Windows: 使用 MSVC 2019+ 编译 Qt 6.x
- Linux: 使用 GCC 9+ 编译 Qt 6.x
- 或者切换到 Qt 5.15（对编译器要求较低）
</details>

<details>
<summary><b>Q: 如何清理构建缓存？</b></summary>

**A:** 删除 `build` 目录：

```bash
# Windows (PowerShell)
Remove-Item -Recurse -Force build

# Linux/macOS
rm -rf build
```

然后重新运行构建脚本。
</details>

<details>
<summary><b>Q: 运行时提示缺少 Qt 动态库？</b></summary>

**A:** 需要将 Qt 的 DLL/SO 文件复制到可执行文件目录，或者：

**Windows:**
```powershell
# 将 Qt bin 目录添加到 PATH
$env:PATH += ";C:\Qt\6.5.3\msvc2019_64\bin"
```

**Linux:**
```bash
export LD_LIBRARY_PATH=/path/to/qt/lib:$LD_LIBRARY_PATH
```

**macOS:**
```bash
export DYLD_LIBRARY_PATH=/path/to/qt/lib:$DYLD_LIBRARY_PATH
```
</details>

<details>
<summary><b>Q: 如何切换 Debug 和 Release 模式？</b></summary>

**A:**

```bash
# 使用脚本
./build.sh Release          # Linux/macOS
.\build.ps1 -BuildType Release  # Windows

# 手动编译
cmake --build build --config Release
```
</details>

---

## 📖 相关文档

- **[CMAKE_MIGRATION.md](CMAKE_MIGRATION.md)** - CMake 迁移指南
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - CMake 快速参考
- **[CMAKE_SUMMARY.md](CMAKE_SUMMARY.md)** - CMake 完整总结
- **[系统架构设计方案](doc/LanChat-Lite%20系统架构设计方案.md)** - 详细的架构设计

---

## 🤝 贡献指南

欢迎贡献代码、报告问题或提出建议！

1. Fork 本仓库
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启一个 Pull Request

---

## 📄 许可证

本项目采用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。

---

## 🌟 致谢

感谢以下开源项目：

- [Qt](https://www.qt.io/) - 跨平台 GUI 框架
- [CMake](https://cmake.org/) - 跨平台构建工具

---

## 📧 联系方式

- 项目地址: [https://github.com/1747982677/LanChat](https://github.com/1747982677/LanChat)
- 问题反馈: [Issues](https://github.com/1747982677/LanChat/issues)

