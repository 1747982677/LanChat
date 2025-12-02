# 从 vcxproj 迁移到 CMake

项目现在已经支持使用 CMake 进行跨平台构建！

##  新增文件

- **CMakeLists.txt** - CMake 配置文件（已更新为匹配当前项目结构）
- **build.ps1** - Windows 快速构建脚本
- **build.sh** - Linux/macOS 快速构建脚本
- **doc/CMake构建指南.md** - 详细的 CMake 使用文档

##  快速开始

### Windows

```powershell
# 方式1：使用自动构建脚本
.\build.ps1

# 如果找不到 Qt，指定 Qt 路径
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"

# 方式2：手动构建
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

### Linux / macOS

```bash
# 方式1：使用自动构建脚本
chmod +x build.sh
./build.sh

# 方式2：手动构建
mkdir build
cd build
cmake ..
make -j$(nproc)
```

##  项目结构

```
LanChat/
├── CMakeLists.txt          # CMake 配置文件
├── build.ps1               # Windows 构建脚本
├── build.sh                # Linux/macOS 构建脚本
├── src/
│   └── main.cpp            # 主程序入口
├── doc/
│   └── CMake构建指南.md    # 详细文档
├── bin/                    # 可执行文件输出目录
└── build/                  # CMake 构建目录（不提交到 Git）
```

##  与 vcxproj 对比

| 特性 | vcxproj | CMake |
|------|---------|-------|
| 跨平台 | 仅 Windows | Windows/Linux/macOS |
| IDE 支持 | Visual Studio | VS/VS Code/CLion/Qt Creator |
| 构建工具 | MSBuild | MSBuild/Make/Ninja |
| 配置文件 | XML (复杂) | 简洁易读 |
| 依赖管理 | 手动配置 | 自动查找 Qt |

##  现状说明

-  **vcxproj 文件仍然保留**，可以继续使用 Visual Studio 打开
-  **CMake 配置已就绪**，可以立即在 Windows/Linux/macOS 上构建
-  **输出目录统一为 `bin/`**
-  **C++14 标准保持不变**
-  **Qt 自动检测** Qt5 或 Qt6

##  详细文档

查看 **[doc/CMake构建指南.md](doc/CMake构建指南.md)** 了解：
- 详细的环境配置
- 跨平台构建步骤
- 常见问题解决
- IDE 集成方法
- 如何添加新文件

##  IDE 推荐

- **Visual Studio 2022** - 原生支持 CMake 项目
- **Visual Studio Code** - 安装 CMake Tools 扩展
- **CLion** - 完美的 CMake 支持
- **Qt Creator** - 可以直接打开 CMakeLists.txt

##  注意事项

1. **不要同时使用 vcxproj 和 CMake 构建**，选择其中一种即可
2. **`build/` 目录已添加到 `.gitignore`**，不会提交到版本控制
3. **首次使用需要配置 Qt 路径**（如果 CMake 自动检测失败）

##  后续步骤

当你添加新的源文件时，需要更新 `CMakeLists.txt` 中的 `SOURCES` 列表：

```cmake
set(SOURCES
    src/main.cpp
    src/新文件.cpp  # 添加这里
)
```
