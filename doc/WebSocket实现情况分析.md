# WebSocket 实现情况分析

## 一、核心组件

### 1. SocketClient (`src/network/socket_client.h/cpp`)

这是 WebSocket 的核心封装类，功能非常完善。

#### ✅ 已实现的功能

**1. 双模式支持**
- ✅ 客户端模式：可以连接到多个 WebSocket 服务器
- ✅ 服务器模式：可以作为 WebSocket 服务器接收连接

**2. 心跳机制（已完整实现）**
- ✅ 每 10 秒自动发送 `ping`
- ✅ 监听 `pong` 响应
- ✅ 连续 3 次未收到 `pong` 则判定断线
- ✅ 自动触发重连机制

**代码位置：**
```cpp
// src/network/socket_client.cpp:181-216
// 心跳定时器：每10秒发送ping
QTimer *hb = new QTimer(this);
hb->setInterval(10000); // 10s
heartbeatTimers[client] = hb;

// 检测pong响应
connect(client, &QWebSocket::pong, this, [this, client](quint64){
    missedPongs[client] = 0;
});

// 心跳超时处理
if (missedPongs[client] >= 3) {
    client->close(); // 触发重连
}
```

**3. 重连机制（已完整实现）**
- ✅ 指数退避策略（1s, 2s, 4s, 8s, 16s, 30s上限）
- ✅ 最大重连次数限制（默认10次，可配置）
- ✅ 自动清理失败连接
- ✅ 连接成功后重置重连计数

**代码位置：**
```cpp
// src/network/socket_client.cpp:218-240, 384-416
// 重连定时器
QTimer *rt = new QTimer(this);
rt->setSingleShot(true);
reconnectTimers[client] = rt;

// 指数退避计算
int shift = qMin(attempts, 6);
int base = 1000;
int cap = 30000;
int delay = qMin(cap, base * (1 << shift));
```

**4. UDP 广播发现（已完整实现）**
- ✅ UDP 端口：45454（默认）
- ✅ 支持局域网广播
- ✅ 支持本机回环（同机多实例）
- ✅ 自动收集在线地址

**代码位置：**
```cpp
// src/network/socket_client.cpp:24-39, 501-559, 561-592
// UDP socket 初始化
discoverySocket = new QUdpSocket(this);
discoverySocket->bind(QHostAddress::AnyIPv4, discoveryPort, ...);

// 广播发现
void SocketClient::broadcastGetOnlineAddresses(int timeoutMs = 1000)
```

**5. 在线地址查询**
- ✅ `online_query` / `online_response` 协议
- ✅ 通过 WebSocket 连接查询在线地址
- ✅ 自动收集响应地址

**代码位置：**
```cpp
// src/network/socket_client.cpp:330-360, 418-452
// 处理 online_query 消息
if (type == QStringLiteral("online_query")) {
    // 回复本端地址
    resp.insert(QStringLiteral("address"), ...);
}
```

**6. 消息收发**
- ✅ 支持文本消息发送
- ✅ 支持 JSON 消息解析
- ✅ 支持按地址发送消息
- ✅ 支持广播消息

**7. 连接管理**
- ✅ 多客户端连接管理
- ✅ 连接状态跟踪
- ✅ 资源自动清理

---

### 2. NetworkWorker (`src/core/network_worker.h/cpp`)

在 Worker 线程中封装 SocketClient，遵循多线程架构。

#### ✅ 已实现的功能

**1. 基本连接管理**
- ✅ `connectToServer(host, port)` - 连接到服务器
- ✅ `disconnectFromServer()` - 断开连接
- ✅ `startServer(port)` - 启动服务器
- ✅ `stopServer()` - 停止服务器

**2. 消息发送**
- ✅ `sendMessage(QJsonObject)` - 发送 JSON 消息
- ✅ `sendTextMessage(QString)` - 发送文本消息
- ✅ 消息发送成功/失败信号

**3. 信号连接**
- ✅ 连接状态信号
- ✅ 消息接收信号
- ✅ 错误信号

**代码位置：**
```cpp
// src/core/network_worker.cpp:26-33
connect(m_socketClient, &SocketClient::connected, ...);
connect(m_socketClient, &SocketClient::messageReceived, ...);
```

---

### 3. NetworkController (`src/core/network_controller.h/cpp`)

在主线程中提供接口，转发请求到 Worker 线程。

#### ✅ 已实现的功能

**1. 接口封装**
- ✅ 所有 NetworkWorker 的方法都有对应接口
- ✅ 信号槽正确连接和转发

**2. 线程安全**
- ✅ 通过信号槽进行线程间通信
- ✅ 符合 Controller-Worker 架构

---

## 二、功能完整性评估

### ✅ 已完整实现（任务9,12要求）

| 功能 | 状态 | 说明 |
|------|------|------|
| WebSocket 心跳 | ✅ 完整 | 10秒间隔，3次未响应断线 |
| 断线重连 | ✅ 完整 | 指数退避，最多10次 |
| UDP 广播心跳 | ✅ 完整 | 端口45454，局域网发现 |
| 在线状态检测 | ✅ 完整 | online_query/response 协议 |
| 消息收发 | ✅ 完整 | 支持 JSON 和文本 |

### ⚠️ 需要扩展的功能（任务9,10,11要求）

| 功能 | 状态 | 说明 |
|------|------|------|
| 状态推送 | ⚠️ 需扩展 | 需要实现 `status_update` 消息推送 |
| 好友请求推送 | ⚠️ 需扩展 | 需要实现 `friend_request` 消息推送 |
| 消息已读通知 | ⚠️ 需扩展 | 需要实现 `message_read` 消息推送 |
| 消息状态反馈 | ⚠️ 需扩展 | 需要完善消息发送状态的回调机制 |

---

## 三、需要扩展的部分

### 1. 状态推送机制

**当前状态：** SocketClient 可以发送消息，但没有专门的状态推送接口。

**需要添加：**
```cpp
// 在 NetworkWorker 中添加
void pushUserStatus(const QString& userId, int status);
void pushFriendRequest(const QJsonObject& requestInfo);
void pushMessageRead(const QString& messageId, const QString& userId);
```

### 2. 消息类型处理

**当前状态：** 所有消息都通过 `messageReceived` 信号发出，没有按类型分发。

**需要添加：**
- 在 `NetworkWorker::onSocketMessageReceived()` 中解析消息类型
- 根据类型分发到不同的处理逻辑：
  - `chat_message` → 保存到数据库
  - `friend_request` → 保存好友请求
  - `status_update` → 更新好友状态
  - `message_read` → 更新消息已读状态

### 3. 消息发送状态跟踪

**当前状态：** `sendMessage()` 立即返回成功，没有等待服务器确认。

**需要改进：**
- 发送消息后等待服务器 `ack` 响应
- 根据响应更新消息状态（Sent/Failed）
- 超时处理（如5秒未收到响应则标记为失败）

---

## 四、代码质量评估

### ✅ 优点

1. **架构清晰**：严格遵循 Controller-Worker 多线程模式
2. **功能完整**：心跳、重连、UDP 发现都已实现
3. **错误处理**：有完善的错误日志和信号
4. **资源管理**：自动清理定时器和连接资源
5. **可扩展性**：接口设计良好，易于扩展

### ⚠️ 需要注意

1. **消息类型处理**：目前所有消息都统一处理，需要按类型分发
2. **状态同步**：需要与数据库层配合，实现状态同步
3. **消息确认**：需要实现消息发送的确认机制

---

## 五、总结

### 已完成的核心功能（约 90%）

✅ **WebSocket 基础功能** - 100% 完成
- 客户端/服务器模式
- 心跳机制
- 重连机制
- UDP 广播发现

✅ **架构设计** - 100% 完成
- Controller-Worker 模式
- 线程安全
- 信号槽通信

### 需要扩展的功能（约 10%）

⚠️ **业务逻辑集成** - 需要实现
- 状态推送接口
- 消息类型分发
- 消息确认机制
- 与数据库层配合

---

## 六、建议的扩展方案

### 方案1：在 NetworkWorker 中添加业务方法

```cpp
// network_worker.h
public slots:
    void pushUserStatus(const QString& userId, int status);
    void pushFriendRequest(const QJsonObject& requestInfo);
    void pushMessageRead(const QString& messageId, const QString& userId);
```

### 方案2：增强消息类型处理

```cpp
// network_worker.cpp
void NetworkWorker::onSocketMessageReceived(const QString& message, const QString& from)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();
    
    if (type == "chat_message") {
        // 转发给 DbLogicWorker 保存
        emit chatMessageReceived(obj, from);
    } else if (type == "friend_request") {
        // 转发给 DbLogicWorker 处理
        emit friendRequestReceived(obj, from);
    } else if (type == "status_update") {
        // 转发给 DbLogicWorker 更新状态
        emit statusUpdateReceived(obj, from);
    }
    // ...
}
```

### 方案3：实现消息确认机制

```cpp
// 发送消息时添加超时检测
void NetworkWorker::sendMessage(const QJsonObject& message)
{
    QString messageId = message.value("messageId").toString();
    
    // 发送消息
    m_socketClient->sendMessageToServer(0, jsonStr);
    
    // 启动超时定时器（5秒）
    QTimer* ackTimer = new QTimer(this);
    ackTimer->setSingleShot(true);
    ackTimer->setInterval(5000);
    
    connect(ackTimer, &QTimer::timeout, this, [this, messageId]() {
        // 超时未收到确认，标记为失败
        emit messageSendFailed(messageId, "Timeout waiting for ack");
    });
    
    // 收到 ack 后停止定时器
    // ...
}
```

---

## 结论

**WebSocket 部分的核心功能已经非常完善**，包括：
- ✅ 心跳机制
- ✅ 重连机制  
- ✅ UDP 广播
- ✅ 多连接管理

**需要扩展的主要是业务逻辑集成**：
- ⚠️ 状态推送接口
- ⚠️ 消息类型分发
- ⚠️ 消息确认机制

这些扩展工作相对简单，主要是：
1. 在 NetworkWorker 中添加几个业务方法
2. 增强消息接收处理逻辑
3. 与 DbLogicWorker 配合实现数据持久化

