# Controller-Worker 架构说明

## 📋 核心概念

这是一个**多线程架构模式**，用于将耗时操作（如数据库操作）从主线程分离，避免阻塞 UI。

## 🎯 两者的作用

### **Controller（控制器）**
- **运行位置**：**主线程**（UI 线程）
- **主要职责**：
  1. **接收外部请求**：从 UI 层或 Service 层接收调用
  2. **参数预处理**：在发送给 Worker 前进行一些轻量级处理（如密码哈希）
  3. **转发请求**：通过 Qt 信号将任务发送到 Worker 线程
  4. **转发结果**：接收 Worker 的结果信号，再转发给调用者
  5. **线程管理**：负责创建和管理 Worker 线程

### **Worker（工作线程）**
- **运行位置**：**独立的子线程**
- **主要职责**：
  1. **执行耗时操作**：数据库查询、文件读写、网络请求等
  2. **业务逻辑处理**：实际的业务逻辑实现
  3. **返回结果**：通过 Qt 信号将结果发送回 Controller

## 🔄 工作流程示例

以**用户注册**为例：

```
┌─────────────┐
│  AuthService │  (主线程)
└──────┬───────┘
       │ 1. registerUser(email, password)
       ▼
┌──────────────────┐
│ DbLogicController │  (主线程)
│                  │
│ - 密码哈希处理    │
│ - 参数验证        │
└──────┬───────────┘
       │ 2. emit requestRegisterUser(email, passwordHash)
       │    (Qt 信号，跨线程通信)
       ▼
┌──────────────────┐
│  DbLogicWorker   │  (子线程)
│                  │
│ - 检查邮箱是否存在│
│ - 插入数据库      │
│ - 生成用户ID      │
└──────┬───────────┘
       │ 3. emit userRegistered(success, userId, error)
       │    (Qt 信号，跨线程通信)
       ▼
┌──────────────────┐
│ DbLogicController │  (主线程)
│                  │
│ - 转发结果        │
└──────┬───────────┘
       │ 4. emit userRegistered(success, userId, error)
       ▼
┌─────────────┐
│  AuthService │  (主线程)
└─────────────┘
```

## 💡 为什么需要这种架构？

### 问题：如果都在主线程会怎样？

```cpp
// ❌ 错误示例：在主线程直接操作数据库
void AuthService::registerUser() {
    // 这会阻塞 UI 线程！
    QSqlQuery query;
    query.exec("SELECT ...");  // 如果数据库很慢，界面会卡住
    query.exec("INSERT ...");  // 用户看到界面"冻结"
}
```

### 解决方案：分离到子线程

```cpp
// ✅ 正确示例：使用 Controller-Worker
void AuthService::registerUser() {
    // 立即返回，不阻塞 UI
    controller.registerUser(email, password);
    // UI 可以继续响应用户操作
}

// Worker 在子线程执行，不会阻塞 UI
void DbLogicWorker::registerUser() {
    // 即使这里很慢，UI 也不会卡住
    QSqlQuery query;
    query.exec("SELECT ...");
    query.exec("INSERT ...");
}
```

## 📊 代码示例对比

### Controller 的代码（主线程）

```cpp
// src/core/dblogic_controller.cpp

void DbLogicController::registerUser(const QString& email, const QString& password)
{
    // 1. 在主线程进行轻量级处理（密码哈希）
    QString passwordHash = PasswordUtil::hashPassword(password);
    
    // 2. 通过信号发送到 Worker 线程（非阻塞）
    emit requestRegisterUser(email, passwordHash);
    // ↑ 这里立即返回，不会等待 Worker 完成
}

// 3. 接收 Worker 的结果并转发
void DbLogicController::connectSignals() {
    // Controller -> Worker（发送请求）
    connect(this, &DbLogicController::requestRegisterUser,
            worker, &DbLogicWorker::registerUser);
    
    // Worker -> Controller（接收结果）
    connect(worker, &DbLogicWorker::userRegistered,
            this, &DbLogicController::userRegistered);
}
```

### Worker 的代码（子线程）

```cpp
// src/core/dblogic_worker.cpp

void DbLogicWorker::registerUser(const QString& email, const QString& passwordHash)
{
    // 1. 在子线程执行耗时操作（不会阻塞 UI）
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT userId FROM users WHERE email = :email");
    checkQuery.exec();  // 即使很慢，UI 也不会卡住
    
    // 2. 执行数据库插入
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users ...");
    insertQuery.exec();
    
    // 3. 通过信号返回结果（跨线程通信）
    emit userRegistered(true, userId, QString());
}
```

## 🎨 架构图

```
┌─────────────────────────────────────────┐
│           主线程 (Main Thread)          │
│                                         │
│  ┌──────────────┐                      │
│  │     UI       │                      │
│  └──────┬───────┘                      │
│         │                              │
│  ┌──────▼──────────┐                   │
│  │  AuthService    │                   │
│  └──────┬──────────┘                   │
│         │                              │
│  ┌──────▼──────────────┐              │
│  │ DbLogicController   │              │
│  │                     │              │
│  │ - 参数预处理         │              │
│  │ - 信号转发           │              │
│  └──────┬──────────────┘              │
│         │  Qt 信号槽                    │
└─────────┼──────────────────────────────┘
          │
          │ 跨线程通信
          │
┌─────────┼──────────────────────────────┐
│         │  子线程 (Worker Thread)       │
│         │                              │
│  ┌──────▼──────────────┐              │
│  │  DbLogicWorker      │              │
│  │                     │              │
│  │ - 数据库操作         │              │
│  │ - 业务逻辑处理        │              │
│  │ - 返回结果           │              │
│  └─────────────────────┘              │
│                                         │
│  ┌─────────────────────┐              │
│  │   SQLite 数据库     │              │
│  └─────────────────────┘              │
└─────────────────────────────────────────┘
```

## 🔑 关键点总结

1. **Controller = 主线程的"代理"**
   - 运行在主线程，可以安全访问 UI
   - 负责接收请求和转发结果
   - 进行轻量级预处理

2. **Worker = 子线程的"执行者"**
   - 运行在独立线程，不会阻塞 UI
   - 执行所有耗时操作
   - 通过信号返回结果

3. **通信方式 = Qt 信号槽**
   - Controller → Worker：`emit requestXXX()`
   - Worker → Controller：`emit xxxCompleted()`
   - Qt 自动处理跨线程通信

4. **优势**
   - ✅ UI 不会卡顿
   - ✅ 代码结构清晰
   - ✅ 易于维护和测试

## 📝 实际应用场景

| 操作 | Controller 职责 | Worker 职责 |
|------|----------------|------------|
| 用户注册 | 密码哈希、参数验证 | 检查邮箱、插入数据库 |
| 用户登录 | 参数验证 | 查询用户、验证密码 |
| 加载聊天记录 | 生成 Request ID | 查询数据库、返回消息列表 |
| 搜索消息 | 参数验证 | 执行 SQL 查询、返回结果 |

## ⚠️ 注意事项

1. **不要在主线程直接调用 Worker 的方法**
   ```cpp
   // ❌ 错误
   worker->registerUser(email, password);  // 跨线程直接调用不安全
   
   // ✅ 正确
   emit requestRegisterUser(email, password);  // 使用信号
   ```

2. **Worker 不能直接访问 UI**
   ```cpp
   // ❌ 错误（在 Worker 中）
   label->setText("完成");  // Worker 在子线程，不能直接操作 UI
   
   // ✅ 正确
   emit operationCompleted("完成");  // 通过信号通知 Controller，再由 Controller 更新 UI
   ```

3. **数据传递要使用值传递或 Qt 容器**
   ```cpp
   // ✅ 正确：使用 Qt 容器（自动处理跨线程）
   emit resultReady(QJsonObject());
   emit listLoaded(QJsonArray());
   ```

