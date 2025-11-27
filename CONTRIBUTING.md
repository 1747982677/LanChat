# CONTRIBUTING.md

## 项目贡献与代码规范（持久化层与测试相关）

本文件旨在定义 LanChat 项目中与数据持久化层、线程模型、命名规则以及测试相关的团队约定，便于所有贡献者遵循并保证代码质量与可维护性。

## 一般编码约定

- C++ 标准：本项目使用 C++17。请在提交前确保编译通过。
- 命名风格：
  - 类型（struct/class/enum）：PascalCase，如 `MessageDao`, `DatabaseManager`。
  - 成员函数与变量：camelCase，如 `insertMessage`, `m_dbPath`。
  - 常量：ALL_CAPS 或 constexpr 在 `common` 命名空间下，如 `Constants::DEFAULT_PORT`。
  - 头文件保护符使用全大写下划线风格：`#ifndef MESSAGE_DAO_H`。
- 文件布局：源文件放 `src/` 下，按模块划分子目录（`model/`, `service/`, `utils/` 等）。
- 注释：关键函数须有简短注释，复杂逻辑需单独注释段落。

## 二、持久化层设计约定（SQLite via QtSql）

目标：使用 Qt 的 `QSqlDatabase`/`QSqlQuery` 将 SQLite 集成到基础设施层，提供线程安全、易测试、易维护的 DAO 接口。

1. 结构（目录与核心类）
   - `src/utils/DatabaseManager`（单例）：管理每个线程的 `QSqlDatabase` 连接、初始化 schema、执行 PRAGMA（如 WAL）、提供事务封装。
   - `src/model/*Dao`：数据访问对象（DAO），如 `MessageDao`, `UserDao`。所有 SQL 操作通过 DAO 暴露给上层 `service` 层。
   - `src/service/ChatService`：业务层。负责消息发送/接收的业务流程（先写 DB -> 发网路 -> 根据回执更新 DB -> 通知 UI）。

2. 连接与线程规则
   - Qt 的 `QSqlDatabase` 是基于连接名管理的，不能跨线程共享同一个连接对象。
   - 每个线程必须持有自己唯一的连接名（建议通过线程 id 生成连接名，如 `lanchat_conn_<threadId>`）。
   - `DatabaseManager::database()` 在当前线程首次调用时创建并打开连接；线程退出前应调用 `DatabaseManager::closeConnectionForCurrentThread()` 清理。
   - 所有直接 DB 操作应在非 UI 线程执行（最好集中在一个 `DBWorker` / `DBThreadPool`）。

3. Schema、迁移与性能
   - 初始化时由 `DatabaseManager` 执行 `ensureSchema()` 创建表。
   - 建表后设置 `PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;` 提升写并发与性能。
   - 保存 schema 版本（`meta` 表）并实现迁移脚本（小版本升级执行 SQL ALTER/CREATE）。

4. 事务策略
   - 对于批量写入（例如消息批次导入、会话清理）使用事务（`beginTransaction` / `commit` / `rollback`）。
   - DAO 可提供 `withTransaction()` 辅助 API（可选）。

5. 错误处理与日志
   - 所有 DB 错误必须记录到 `Logger::getInstance()`，并向上层返回明确的错误码或布尔值。
   - 不在 DAO 内弹 UI 弹窗；DAO 仅返回失败状态，由 `service` 层决定如何反馈用户。

## 三、Message 与 ChatService 行为约定

1. Message 模型（示例字段）
   - `id` (INTEGER PK AUTOINCREMENT)
   - `sender` TEXT
   - `receiver` TEXT
   - `content` TEXT
   - `timestamp` INTEGER (UTC epoch)
   - `status` INTEGER (0=Sending,1=Sent,2=Received,3=Failed)
   - `extra` TEXT (可选 JSON 字段)

2. ChatService 流程（主要函数）
   - `sendMessage(const QString& to, const QString& content)`
     1. 校验输入长度与格式。
     2. 构造 `Message`（status=0 Sending），调用 `MessageDao::insertMessage()` 并获取 `id`。
     3. 立即发信号给 UI 显示（含本地 id 和 Sending 状态）。
     4. 将要发送的 payload 传给 `NetworkManager::send()`（最好是 JSON），并附带本地 id 以便回执关联。
     5. `NetworkManager` 在收到 ack 时触发回调：`onSendAck(localId, success)`。
     6. `ChatService` 在 ack 回调中调用 `MessageDao::updateStatus(localId, Sent/Failed)` 并通知 UI 更新状态。

   - `onMessageReceived(const QString& json)`
     1. 解析 JSON 为 Message 结构。
     2. 保存到 DB（status=2 Received）。
     3. 触发 UI 更新（NewMessage signal），并维护未读计数。

3. 可靠性策略
   - 如果网络层报错或无连接，`sendMessage` 返回本地已保存但状态为 `Sending`，并在后台重试（可配置重试次数与间隔），或由用户主动重试。
   - 定期清理失败/过期消息策略可作为后台任务运行。

## 四、测试策略（必需）

目标：DAO、DatabaseManager、ChatService 均需可自动化测试。采用 Qt Test (`QTest`) 或 Google Test (`gtest`)。推荐使用 Qt Test 以便与 Qt 项目无缝集成。

1. 单元测试原则
   - DAO 层使用 SQLite 的内存数据库 `:memory:` 或临时文件进行测试，保证测试互相隔离且速度快。
   - DatabaseManager 应提供测试 hook（例如允许传入 dbPath，或在测试时使用 `:memory:`，并允许重置 schema）。
   - 职责分离：业务逻辑（ChatService）在测试时应 mock `NetworkManager`（用 TestStub 发 ack），以验证 DB 写入与状态更新逻辑。

2. 推荐测试用例
   - `MessageDao`:
     - 插入一条消息 -> 验证返回 id > 0 -> Query 确认数据一致。
     - 更新状态 -> Query 确认状态改变。
     - 批量插入 + 事务回滚行为验证。
   - `DatabaseManager`:
     - init 打开并创建 schema -> 验证表存在与 PRAGMA 设置。
     - 多线程连接：在子线程调用 `database()` 并插入数据 -> 主线程查询可见性验证（WAL 模式）。
   - `ChatService`:
     - sendMessage 路径：模拟 `NetworkManager` 成功 ack -> 验证 message 在 DB 中状态从 `Sending` -> `Sent`，且 UI 信号发出（可用信号捕获断言）。
     - sendMessage 网络失败 -> 验证状态为 `Failed` 或保持 `Sending` 且触发重试逻辑。
     - onMessageReceived：模拟网络推送 -> 验证 DB 中新增消息且 UI 信号发出。

3. 测试实现建议
   - 使用 `QSignalSpy` 捕获信号（UI 通知、ChatService 事件）。
   - 测试隔离：每个测试用例在开始时重新 `DatabaseManager::init()` 指定 `:memory:` 或独立临时 DB 文件，结束时清理连接。
   - CI 集成：在 CI 脚本中运行 `ctest` 或调用测试可执行文件。

## 五、CI 与本地运行

- 在 CMakeLists 中确保链接 Qt Sql 模块（`Qt6::Sql` / `Qt5::Sql`）。
- 在 CI 环境安装 Qt 或使用预装镜像，确保能运行 GUIless 的 Qt Test（`QT_QPA_PLATFORM=offscreen`）。
- 测试命令示例：
  - 本地：`ctest --output-on-failure` 或直接运行测试二进制。
  - CI：在构建脚本中运行 `cmake --build . --config Release` 然后 `ctest -C Release`。

## 六、代码审查要点

- DB 操作必须有错误处理与日志记录。
- 不允许跨线程共享 `QSqlQuery` / `QSqlDatabase` 对象。
- DAO 不直接调用 UI；业务层负责错误转译并通知 UI。
- 提交时包含单元测试覆盖新增逻辑。

---

如需我把这些规范落地成 `CONTRIBUTING.md` 文件并提交到仓库，请告知，我将自动生成并提交。