# LanChat-Lite 系统架构设计方案

## 1. 总体架构视图 

    graph TD
    
        subgraph "客户端 (C++ PC Client)"
    
            UI[UI 展示层 (Main Thread)]
    
            Core[核心业务层 (Controller)]
    
            Infra[基础设施层 (IO Threads)]
    
            LocalDB[(本地 SQLite)]
    
        end
    
    subgraph "服务端 (Node.js/Python)"
        Relay[简易信令转发器]
    end
    
    UI <--> Core
    Core <--> Infra
    Infra <--> LocalDB
    Infra --"WebSocket (JSON)"--> Relay
------

## 2. 详细分层架构 (Layered Architecture)

我们将客户端代码严格分为三层。**规则：上层可以调用下层，下层只能通过“回调/信号”通知上层（严禁下层引用上层头文件）。**

### 第一层：表现层 (Presentation Layer / UI)

- **对应模块：** M2 (UI & 状态管理)
- **运行线程：** 主线程 (Main Thread)
- **职责：** 负责画图、响应鼠标点击、展示数据。**绝不包含任何业务逻辑（如解析 JSON、读写数据库）。**
- **核心组件：**
  - `MainWindow`: 程序外壳。
  - `ChatWidget`: 聊天主窗口。
  - `ContactList`: 联系人列表组件。

### 第二层：业务逻辑层 (Application / Service Layer)

- **对应模块：** **M6 (核心业务 - 新增)**
- **运行线程：** 独立的逻辑线程。
- **职责：** 系统的“大脑”。负责协调 UI、网络和数据库。
- **核心组件：**
  - `ClientContext` (单例): 全局上下文，持有 Service 的指针。
  - `AuthService`: 处理登录、注销、Token 管理。
  - `ChatService`: 核心！处理消息收发逻辑。
    - *动作：* 收到 UI 的“发送”请求 -> 校验 -> 存入 DB (Pending状态) -> 调网络层发送 -> 收到回执 -> 更新 DB (Success状态) -> 刷新 UI。
  - `ContactService`: 管理好友列表数据。

### 第三层：基础设施层 (Infrastructure Layer)

- **对应模块：** M3 (网络), M4 (存储), M5 (日志)
- **运行线程：** **子线程 (Worker Threads)**
- **职责：** 逻辑处理。
- **核心组件：**
  - `NetworkManager`:
    - 封装 WebSocket 库 (如 websocketpp)。
    - 维护连接、心跳、重连。
    - **关键：** 提供 `send(json)` 接口；提供 `onMessage(callback)` 回调。
  - `DatabaseManager`:
    - 封装 SQLite3。
    - 提供 `exec()`, `query()` 等基础接口。
    - 或者封装 DAO (Data Access Object)，如 `UserDao`, `MessageDao`。
  - `ConfigManager`: 读取本地 .ini/.json 配置文件。
  - `LogManager`: 基于 spdlog，提供全局日志宏。

------

## 3. 关键数据流 (Data Flow Sequence)

### 场景一：发送消息 (UI -> Net)

1. **UI层**: 用户点击发送，调用 `ChatService::sendMessage(text)`。
2. **Service层**:
   - 构造一个 `Message` 对象。
   - 调用 `MessageDao::save(msg)` 将消息写入本地数据库（状态：Sending）。
   - **通知 UI**: 界面立刻显示这条消息（此时显示个转圈圈/灰色的发送中图标）。
   - 调用 `NetworkManager::send(json)`。
3. **Infra层 (Net)**: 将 JSON 扔进 WebSocket 队列，通过网络发出。
4. **回调**:
   - 如果网络层收到服务器的 `ack` (回执)。
   - `NetworkManager` 触发回调 -> `ChatService` 更新 DB 状态为 "Sent"。
   - `ChatService` 通知 UI -> 界面上的转圈圈消失。

### 场景二：接收消息 (Net -> UI)

1. **Infra层 (Net)**: WebSocket 线程收到一段 JSON 数据。
2. **Infra层**: 通过回调函数/信号槽，把数据抛给 `ChatService`。**（注意：这里通常需要切换到主线程，或者加锁）**。
3. **Service层**:
   - `ChatService` 解析 JSON，发现是 `type: "chat_msg"`。
   - 调用 `MessageDao::save(msg)` 存入本地数据库。
   - 判断当前 UI 打开的是不是发信人的窗口？
     - 是：触发 `NewMessage` 事件。
     - 否：触发 `UnreadCountUpdate` 事件（未读数+1）。
4. **UI层**: 监听到事件，刷新列表，渲染新气泡。

------

## 4. 工程目录结构 (Directory Structure)

结构去创建文件夹（S7 模块的工作）：

LanChat/
├── bin/                  # 编译输出目录
├── doc/                  # 设计文档
├── src/
│   ├── main.cpp          # 程序入口
│   ├── common/           # 通用定义 (S8)
│   │   ├── global.h      # 错误码、常量
│   │   └── types.h       # 核心结构体 (User, Message)
│   ├── ui/               # 表现层 (M2, S1, S2)
│   │   ├── main_window/
│   │   ├── chat_window/
│   │   └── assets/       # 图标、样式表
│   ├── service/          # 业务逻辑层 (M6, S9)
│   │   ├── auth_service.h/.cpp
│   │   └── chat_service.h/.cpp
│   ├── model/            # 数据实体与DAO (M4, S5)
│   │   ├── user_dao.h
│   │   └── message_dao.h
│   ├── network/          # 网络层 (M3, S3, S4)
│   │   ├── socket_client.h
│   │   └── packet_parser.h
│   └── utils/            # 工具类 (M5, S6)
│       ├── db_helper.h   # SQLite 封装
│       ├── logger.h      # 日志
│       └── config.h      # 配置读取
├── tests/                # 测试用例
├── third_party/          # 第三方库 (json, sqlite3, websocketpp, spdlog)
└── CMakeLists.txt        # 构建脚本

------

## 5. 核心技术决策 

1. **线程通信机制 (Thread Inter-process Communication):**

   - 

2. **数据模型 (Model):**

   - 不要在 UI 代码里直接操作 JSON。
   - **强制要求：** 在 `common/types.h` 里定义 struct。

3. **数据库连接管理**（或者说客户端本地数据存储）

   

