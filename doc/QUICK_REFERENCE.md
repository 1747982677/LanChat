# CMake 快速参考

##  快速命令

### Windows
```powershell
# 最简单 - 使用自动脚本
.\build.ps1

# 指定 Qt 路径
.\build.ps1 -QtPath "C:\Qt\6.5.3\msvc2019_64"

# 手动构建 - 完整流程
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
cd ..
.\bin\LanChat.exe
```

### Linux
```bash
# 最简单 - 使用自动脚本
chmod +x build.sh && ./build.sh

# 手动构建 - 完整流程
mkdir build && cd build
cmake ..
make -j$(nproc)
cd ..
./bin/LanChat
```

### macOS
```bash
# 最简单 - 使用自动脚本
chmod +x build.sh && ./build.sh

# 手动构建 - 完整流程
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
make -j$(sysctl -n hw.ncpu)
cd ..
./bin/LanChat
```

##  项目文件说明

| 文件 | 用途 | 是否提交 Git |
|------|------|-------------|
| `CMakeLists.txt` | CMake 配置文件 | 是 |
| `build.ps1` | Windows 构建脚本 | 是 |
| `build.sh` | Linux/macOS 构建脚本 | 是 |
| `build/` | 构建目录 | 否 |
| `bin/` | 可执行文件输出 | 否 |
| `LanChat.vcxproj*` | Visual Studio 项目 | 可选 |

##  常用 CMake 命令

```bash
# 配置项目
cmake -B build

# 编译
cmake --build build

# Debug 模式
cmake --build build --config Debug

# Release 模式
cmake --build build --config Release

# 清理构建
rm -rf build  # Linux/macOS
Remove-Item -Recurse -Force build  # Windows

# 安装
cmake --install build

# 查看帮助
cmake --help
```

##  IDE 使用

### Visual Studio 2022
1. 文件 → 打开 → 文件夹
2. 选择项目根目录
3. 自动识别 CMakeLists.txt
4. 按 F5 调试

### Visual Studio Code
1. 安装 "CMake Tools" 扩展
2. 打开项目文件夹
3. `Ctrl+Shift+P` → "CMake: Configure"
4. `F7` 编译

### CLion
1. 打开项目根目录
2. 自动加载 CMakeLists.txt
3. 直接运行/调试

##  故障排除

### 找不到 Qt
```bash
cmake .. -DCMAKE_PREFIX_PATH="Qt安装路径"
```

### 切换 Qt 版本
```bash
# 优先使用 Qt6
cmake .. -DCMAKE_PREFIX_PATH="/path/to/Qt6"

# 强制使用 Qt5
cmake .. -DCMAKE_PREFIX_PATH="/path/to/Qt5"
```

### 查看详细构建信息
```bash
cmake --build build --verbose
```

### 重新配置
```bash
rm -rf build
cmake -B build
```

##  文档索引

- **CMAKE_MIGRATION.md** - 迁移说明和快速开始
- **CMAKE_SUMMARY.md** - 完整总结
- **doc/CMake构建指南.md** - 详细使用指南
- **QUICK_REFERENCE.md** - 本文档

##  提示

-  构建目录（`build/`）不要提交到 Git
-  首次运行可能需要指定 Qt 路径
-  修改 CMakeLists.txt 后重新运行 cmake
-  跨平台开发时保持配置文件同步

---

**更多帮助**: 查看 `doc/CMake构建指南.md`
