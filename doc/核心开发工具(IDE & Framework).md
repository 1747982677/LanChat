### 1. 核心开发环境 (IDE & Framework)

| **工具名称**               | **推荐版本**   | **必选组件/说明**                            | **核心用途**                                                 |
| -------------------------- | -------------- | -------------------------------------------- | ------------------------------------------------------------ |
| **Visual Studio**          | **2022 **      | 勾选工作负载：**"使用 C++ 的桌面开发"**      | **编写代码、编译、调试**。                                   |
| **Qt Framework**           | **Qt 6.5 LTS** | 安装时勾选：**Qt 6.5.x -> MSVC 2019 64-bit** | **核心框架**。6.5 是长期支持版，稳定且支持高分屏。包含了 UI、网络、数据库所有功能。 |
| **Qt Visual Studio Tools** | **(插件)**     | 在 VS 扩展商店里搜索安装                     | **连接器**。让 VS 能识别 Qt 工程，一键创建 Qt 项目，无需手写 CMake。 |
| **CMake**                  | **3.24+**      | VS2022 自带，无需单独安装                    | **构建系统**。Qt 6 官方推荐的标准构建工具。                  |
| **Git**                    | **2.40+**      | -                                            | **代码版本管理**。                                           |

------

### 2. 辅助与测试工具 (Utility)

| **工具名称**     | **推荐版本** | **说明** | **核心用途**                                                 |
| ---------------- | ------------ | -------- | ------------------------------------------------------------ |
| **SQLiteStudio** | **3.3+**     | 免费开源 | **数据库可视化**。用来打开生成的 `.db` 文件，查看消息有没有存进去，验证 SQL 写得对不对。 |
| **Node.js**      | **v18 LTS**  | -        | **运行简易服务端**。                                         |
| **Postman**      |              | -        |                                                              |

------

### 3. 架构模块对应的 Qt 类映射表

| **你的架构模块** | **原计划 (C++ 标准库)** | **All-in Qt 方案 (实际使用类)**     | **优势**                                                     |
| ---------------- | ----------------------- | ----------------------------------- | ------------------------------------------------------------ |
| **M1 工程**      | `CMakeLists.txt`        | `CMakeLists.txt` (Qt版)             | 自动管理 `find_package(Qt6 ...)`                             |
| **M2 UI**        | 纯 C++ / ImGui          | **QWidget, QMainWindow, QListView** | 原生控件，自带样式表 (QSS) 支持                              |
| **M3 网络**      | websocketpp + asio      | **QWebSocket**                      | 信号槽机制：`connect(socket, &QWebSocket::textMessageReceived, ...)` 极其优雅 |
| **M4 存储**      | sqlite3.h               | **QSqlDatabase, QSqlQuery**         | 自动处理类型转换                                             |
| **S5 数据模型**  | `std::string`, `struct` | **QString, QJsonObject**            | **QJsonDocument** 直接转 JSON 字符串                         |
| **M5 日志**      | spdlog                  | **QDebug, qInstallMessageHandler**  | `qDebug() << "Login success";`                               |
| **线程模型**     | `std::thread`           | **QThread, QtConcurrent**           | 安全地把耗时任务扔到后台                                     |

------

### 4. 环境安装 (Action Plan)

1. **安装 VS 2022**
   - 安装时勾选 **"Desktop development with C++"** (使用 C++ 的桌面开发)。
2. **注册 Qt 账号并下载安装器**
   - 去 [Qt.io](https://www.google.com/search?q=https://www.qt.io/download-qt-installer) 下载 "Qt Online Installer"。
3. **安装 Qt 6.5**
   - 登录安装器。
   - 选择 **Custom Installation**。
   - 展开 **Qt** -> **Qt 6.5.3** (或最新 6.5.x)。
   - **关键勾选：** `MSVC 2019 64-bit` (VS2022 兼容 2019 的二进制)。
   - *可选：* `Qt 5 Compatibility Module` (为了保险可以勾上)。
   - 其他 Android/WebAssembly 统统不要勾（省几十 G 空间）。
4. **配置 VS 插件**
   - 打开 VS 2022 -> 菜单栏 `Extensions` (扩展) -> `Manage Extensions`。
   - 搜索 **"Qt Visual Studio Tools"** -> 下载 -> 重启 VS 安装。
   - 重启后，VS 菜单栏会出现 `Qt` 选项。
   - 点击 `Qt` -> `Qt Versions` -> 点击文件夹图标 -> 找到你安装的 Qt 路径 (例如 `C:\Qt\6.5.3\msvc2019_64\bin\qmake.exe`) -> 确定。