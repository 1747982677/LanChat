#ifndef DBLOGIC_CONTROLLER_H
#define DBLOGIC_CONTROLLER_H

#include "core/base_controller.h"
#include <QJsonObject>
#include <QJsonArray>
#include "model/message.h"
#include "ui/personinfo/UserEntity.h"

/**
 * @brief 数据库与业务逻辑控制器
 * 
 * 运行在主线程，负责：
 * 1. 管理 DbLogicWorker 的生命周期
 * 2. 提供数据库操作的外部接口
 * 3. 转发数据事件到上层
 */
class DbLogicController : public BaseController
{
    Q_OBJECT

public:
    explicit DbLogicController(QObject* parent = nullptr);
    ~DbLogicController() override;

    bool initialize() override;

    /**
     * @brief 获取单例
     */
    static DbLogicController& instance();

    /**
     * @brief 检查数据库是否已初始化
     * @return 是否已初始化
     */
    bool isDatabaseInitialized() const;

public slots:
    /**
     * @brief 初始化数据库
     */
    void initializeDatabase(const QString& dbPath);

    /**
     * @brief 保存消息
     */
    void saveMessage(const QJsonObject& message);
    
    //  修复：返回 requestId，UI 可以用来验证
    QString loadHistoryMessages(const QString& contactId, int limit = 50, int offset = 0);
    
    /**
     * @brief 搜索消息
     */
    void searchMessages(const QString& keyword);

    /**
     * @brief 更新消息状态
     */
    void updateMessageStatus(const QString& messageId, const QString& status);

    /**
     * @brief 删除消息
     */
    void deleteMessage(const QString& messageId);

    /**
     * @brief 加载联系人列表
     */
    void loadContactList();

    /**
     * @brief 添加联系人
     */
    void addContact(const QJsonObject& contactInfo);

    /**
     * @brief 更新联系人
     */
    void updateContact(const QString& contactId, const QJsonObject& contactInfo);

    /**
     * @brief 处理文件
     */
    void processFile(const QString& filePath, const QJsonObject& options);

    /**
     * @brief 通过账号搜索用户（精确匹配，不支持模糊查询）
     * @param account 账号（邮箱）
     */
    void searchUserByAccount(const QString& account);

    /**
     * @brief 注册新用户
     * @param email 邮箱（账号）
     * @param password 明文密码（将在内部进行哈希）
     */
    void registerUser(const QString& email, const QString& password);

    /**
     * @brief 验证用户密码
     * @param email 邮箱（账号）
     * @param password 明文密码
     */
    void verifyUserPassword(const QString& email, const QString& password);

    //***以下关于lanchat/messages数据表的设计****
    void queryMessages(const QString& localUser, const QString& peer, int limit);

    //*********关于public/user数据表的设计********
    void queryUser(const UserEntity& localUser);
    void updateUser(const UserEntity& localUser);
    void addUser(const UserEntity& localUser);

signals:
    // 发送给 Worker 的信号
    void requestInitializeDatabase(const QString& dbPath);
    void requestSaveMessage(const QJsonObject& message);
    
    //  修复：请求信号包含 requestId
    void requestLoadHistoryMessages(const QString& requestId, const QString& contactId, int limit, int offset);
    
    void requestSearchMessages(const QString& keyword);
    void requestUpdateMessageStatus(const QString& messageId, const QString& status);
    void requestDeleteMessage(const QString& messageId);
    void requestLoadContactList();
    void requestAddContact(const QJsonObject& contactInfo);
    void requestUpdateContact(const QString& contactId, const QJsonObject& contactInfo);
    void requestProcessFile(const QString& filePath, const QJsonObject& options);
    void requestSearchUserByAccount(const QString& account);
    void requestRegisterUser(const QString& email, const QString& passwordHash);
    void requestVerifyUserPassword(const QString& email, const QString& password);

    // 从 Worker 接收的信号（转发）
    void databaseInitialized(bool success);
    void messageSaved(bool success, const QString& messageId);
    
    //  修复：响应信号包含 requestId
    void historyMessagesLoaded(const QString& requestId, const QJsonArray& messages, const QString& contactId);
    
    void searchResultsReady(const QJsonArray& results);
    void messageStatusUpdated(bool success, const QString& messageId);
    void contactListLoaded(const QJsonArray& contacts);
    void contactOperationCompleted(bool success, const QString& operation);
    void fileProcessed(bool success, const QString& filePath, const QString& resultPath);
    void userSearchResult(const QJsonObject& userInfo, bool found);
    void userRegistered(bool success, const QString& userId, const QString& errorMessage);
    void passwordVerified(bool success, const QString& userId, const QString& errorMessage);

    //***以下关于lanchat/messages数据表的设计****
    void requestQueryMessages(const QString& localUser, const QString& peer, int limit);
    void queryResultsReady(const QVector<Message>& results);

    //*********关于public/user数据表的设计********
    void requestQueryUser(const UserEntity& localUser);
    void queryUserReady(const UserEntity& results);

    void requesUpdateUser(const UserEntity& localUser);
    void updateUserReady(const bool& glag);

    void requesAddUser(const UserEntity& localUser);
    void addUserReady(const bool& glag);

protected:
    QObject* createWorker() override;

private:
    void connectSignals();
    static DbLogicController* s_instance;
    
    // 数据库初始化状态
    bool m_dbInitialized;
    QString m_dbPath;  // 保存数据库路径，以便在需要时触发初始化
};

#endif // DBLOGIC_CONTROLLER_H
