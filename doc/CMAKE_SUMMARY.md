# LanChat - CMake 迁移完成总结

##  已完成的工作

### 1. 更新 CMakeLists.txt
-  适配当前项目结构（只包含 main.cpp）
-  支持 Qt5 和 Qt6 自动检测
-  配置 C++14 标准
-  设置输出目录为 `bin/`
-  Windows 子系统配置（无控制台窗口）
-  跨平台编译器警告设置

### 2. 创建构建脚本
-  **build.ps1** - Windows PowerShell 自动化构建脚本
-  **build.sh** - Linux/macOS Bash 自动化构建脚本

### 3. 更新 .gitignore
-  添加 CMake 构建目录忽略规则
-  添加 Qt 自动生成文件忽略规则

### 4. 创建文档
-  **doc/CMake构建指南.md** - 详细的跨平台构建指南
-  **CMAKE_MIGRATION.md** - 迁移说明和快速开始指南
-  **CMAKE_SUMMARY.md** - 本总结文档

### 在 Windows 上
```powershell
# 使用自动脚本
.\build.ps1

# 或手动构建
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

### 在 Linux 上
```bash
chmod +x build.sh
./build.sh

# 或手动构建
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 在 macOS 上
```bash
chmod +x build.sh
./build.sh

# 或手动构建
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
make -j$(sysctl -n hw.ncpu)
```

##  对比优势

| 方面 | 之前 (vcxproj) | 现在 (CMake) |
|------|----------------|--------------|
| **平台支持** | 仅 Windows | Windows + Linux + macOS |
| **IDE 支持** | Visual Studio | VS + VS Code + CLion + Qt Creator |
| **配置文件** | XML，冗长复杂 | 简洁的 CMake 脚本 |
| **依赖管理** | 手动设置路径 | 自动查找 Qt |
| **构建系统** | MSBuild | MSBuild / Make / Ninja |
| **跨平台协作** | 困难 | 简单 |

##  迁移策略

你有两个选择：

### 选项 1：保留双构建系统（推荐过渡期使用）
-  vcxproj 和 CMake 都保留
-  在 Windows 上可以继续使用熟悉的 Visual Studio
-  同时获得跨平台能力
-  添加新文件时需要同时更新两个配置

### 选项 2：完全切换到 CMake（推荐长期使用）
-  统一的构建系统
-  简化项目维护
-  更好的跨平台协作
-  可以删除 `.sln`、`.vcxproj`、`.vcxproj.filters` 文件

##  后续开发注意事项

### 添加新的源文件
编辑 `CMakeLists.txt`，在 `SOURCES` 中添加：
```cmake
set(SOURCES
    src/main.cpp
    src/new_file.cpp  # 新文件
)
```

### 添加新的 Qt 模块
如果需要使用更多 Qt 模块（如 Network、Sql），修改：
```cmake
find_package(Qt6 COMPONENTS Core Widgets Network Sql QUIET)
# ...
target_link_libraries(LanChat PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)
```

### 添加第三方库
```cmake
# 查找库
find_package(SomeLibrary REQUIRED)

# 链接库
target_link_libraries(LanChat PRIVATE SomeLibrary::SomeLibrary)
```

##  常见问题

### Q: 找不到 Qt？
**A:** 使用 `-DCMAKE_PREFIX_PATH` 指定 Qt 路径：
```powershell
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"
```

### Q: CMake 和 vcxproj 可以共存吗？
**A:** 可以！但建议只使用一个系统，避免配置不同步。

### Q: 如何在 Visual Studio 中使用 CMake？
**A:** Visual Studio 2022 原生支持 CMake：
1. 文件 → 打开 → 文件夹
2. 选择项目根目录
3. VS 会自动识别 CMakeLists.txt
