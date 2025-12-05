#ifndef DBLOGIC_WORKER_H
#define DBLOGIC_WORKER_H

#include "core/base_worker.h"
#include <QString>
#include <QJsonObject>
#include "model/message.h"
#include "ui/personinfo/UserEntity.h"

/**
 * @brief 数据库与业务逻辑 Worker
 * 
 * 运行在独立业务逻辑线程中，负责：
 * 1. 数据库增删改查操作
 * 2. 耗时业务逻辑处理
 * 3. 数据格式转换
 * 4. 历史记录、文件处理等
 */
class DbLogicWorker : public BaseWorker
{
    Q_OBJECT

public:
    explicit DbLogicWorker(QObject* parent = nullptr);
    ~DbLogicWorker() override;

    bool initialize() override;
    void cleanup() override;

public slots:
    /**
     * @brief 初始化数据库
     * @param dbPath 数据库路径
     */
    void initializeDatabase(const QString& dbPath);

    /**
     * @brief 保存消息到数据库
     * @param message 消息对象
     */
    void saveMessage(const QJsonObject& message);

    /**
     * @brief 加载历史消息
     * @param requestId 请求ID（用于异步响应匹配）
     * @param contactId 联系人 ID
     * @param limit 最大返回数量
     * @param offset 偏移量
     */
    void loadHistoryMessages(const QString& requestId, const QString& contactId, int limit, int offset);
    
    /**
     * @brief 搜索消息
     * @param keyword 搜索关键字
     */
    void searchMessages(const QString& keyword);

    /**
     * @brief 通过账号搜索用户（精确匹配，不支持模糊查询）
     * @param account 账号
     */
    void searchUserByAccount(const QString& account);

    /**
     * @brief 注册新用户
     * @param email 邮箱（账号）
     * @param passwordHash 密码哈希值
     */
    void registerUser(const QString& email, const QString& passwordHash);

    /**
     * @brief 验证用户密码
     * @param email 邮箱（账号）
     * @param password 明文密码
     */
    void verifyUserPassword(const QString& email, const QString& password);
    
    /**
     * @brief 发送好友请求
     * @param senderId 发送者用户ID
     * @param receiverId 接收者用户ID
     * @param senderAccount 发送者账号（邮箱）
     * @param senderNickname 发送者昵称
     * @param avatarPath 发送者头像路径
     * @param verifymsg 验证消息（可选）
     */
    void sendFriendRequest(const QString& senderId, const QString& receiverId,
                          const QString& senderAccount, const QString& senderNickname,
                          const QString& avatarPath, const QString& verifymsg = QString());
    
    /**
     * @brief 查询收到的好友请求（状态为 Pending）
     * @param receiverId 接收者用户ID
     */
    void queryFriendRequests(const QString& receiverId);
    
    /**
     * @brief 接受好友请求
     * @param requestId 请求ID
     * @param senderId 发送者用户ID
     * @param receiverId 接收者用户ID
     */
    void acceptFriendRequest(const QString& requestId, const QString& senderId, const QString& receiverId);

    /**
     * @brief 更新消息状态
     * @param messageId 消息 ID
     * @param status 状态（如：pending, sent, delivered, read等）
     */
    void updateMessageStatus(const QString& messageId, const QString& status);

    /**
     * @brief 删除消息
     * @param messageId 消息 ID
     */
    void deleteMessage(const QString& messageId);

    /**
     * @brief 加载联系人列表
     * @param userId 当前用户ID
     */
    void loadContactList(const QString& userId);

    /**
     * @brief 添加联系人
     * @param contactInfo 联系人信息
     */
    void addContact(const QJsonObject& contactInfo);

    /**
     * @brief 更新联系人信息
     * @param contactId 联系人 ID
     * @param contactInfo 联系人信息
     */
    void updateContact(const QString& contactId, const QJsonObject& contactInfo);

    /**
     * @brief 处理文件（压缩、转换等）
     * @param filePath 文件路径
     * @param options 处理选项
     */
    void processFile(const QString& filePath, const QJsonObject& options);

    /**
     * @brief 查询消息（用于lanchat/messages数据表）
     * @param localUser 本地用户ID
     * @param peer 对方用户ID
     * @param limit 限制数量
     */
    void queryMessages(const QString& localUser, const QString& peer, int limit);

    /**
     * @brief 查询用户（用于public/user数据表）
     * @param localUser 用户实体
     */
    void queryUser(const UserEntity& localUser);

    /**
     * @brief 更新用户（用于public/user数据表）
     * @param localUser 用户实体
     */
    void updateUser(const UserEntity& localUser);

    /**
     * @brief 添加用户（用于public/user数据表）
     * @param localUser 用户实体
     */
    void addUser(const UserEntity& localUser);

signals:
    /**
     * @brief 数据库初始化完成
     */
    void databaseInitialized(bool success);

    /**
     * @brief 消息保存完成
     */
    void messageSaved(bool success, const QString& messageId);

    /**
     * @brief 历史消息加载完成
     * @param requestId 请求ID（用于异步响应匹配）
     * @param messages 消息列表（JSON 数组）
     * @param contactId 联系人 ID
     */
    void historyMessagesLoaded(const QString& requestId, const QJsonArray& messages, const QString& contactId);

    /**
     * @brief 消息搜索结果
     * @param results 搜索结果（JSON 数组）
     */
    void searchResultsReady(const QJsonArray& results);
    
    /**
     * @brief 消息状态更新完成
     */
    void messageStatusUpdated(bool success, const QString& messageId);

    /**
     * @brief 联系人列表加载完成
     */
    void contactListLoaded(const QJsonArray& contacts);

    /**
     * @brief 联系人操作完成
     */
    void contactOperationCompleted(bool success, const QString& operation);

    /**
     * @brief 文件处理完成
     */
    void fileProcessed(bool success, const QString& filePath, const QString& resultPath);

    /**
     * @brief 用户搜索结果
     */
    void userSearchResult(const QJsonObject& userInfo, bool found);

    /**
     * @brief 用户注册结果
     * @param success 是否成功
     * @param userId 用户ID（成功时）
     * @param errorMessage 错误信息（失败时）
     */
    void userRegistered(bool success, const QString& userId, const QString& errorMessage);

    /**
     * @brief 密码验证结果
     * @param success 是否验证成功
     * @param userId 用户ID（成功时）
     * @param errorMessage 错误信息（失败时）
     */
    void passwordVerified(bool success, const QString& userId, const QString& errorMessage);

    /**
     * @brief 查询消息结果（用于lanchat/messages数据表）
     */
    void queryResultsReady(const QVector<Message>& results);

    /**
     * @brief 查询用户结果（用于public/user数据表）
     */
    void queryUserReady(const UserEntity& localUser);

    /**
     * @brief 更新用户结果（用于public/user数据表）
     */
    void updateUserReady(const bool& glag);

    /**
     * @brief 添加用户结果（用于public/user数据表）
     */
    void addUserReady(const bool& glag);

    /**
     * @brief 好友请求发送结果
     * @param success 是否成功
     * @param requestId 请求ID（可选，若有）
     * @param errorMessage 错误信息（失败时）
     */
    void friendRequestSent(bool success, const QString& requestId, const QString& errorMessage);
    
    /**
     * @brief 好友请求查询结果
     * @param requests 好友请求列表（JSON数组）
     */
    void friendRequestsLoaded(const QJsonArray& requests);
    
    /**
     * @brief 好友请求接受结果
     * @param success 是否成功
     * @param errorMessage 错误信息（失败时）
     */
    void friendRequestAccepted(bool success, const QString& errorMessage);

private:
    QString m_dbPath;
    bool m_dbInitialized;
};

#endif // DBLOGIC_WORKER_H
