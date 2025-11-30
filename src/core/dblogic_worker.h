#ifndef DBLOGIC_WORKER_H
#define DBLOGIC_WORKER_H

#include "core/base_worker.h"
#include <QString>
#include <QJsonObject>

/**
 * @brief 数据库与业务逻辑 Worker
 * 
 * 运行在独立的逻辑线程中，负责：
 * 1. 数据库的增删改查操作
 * 2. 耗时的业务逻辑处理
 * 3. 数据解析和转换
 * 4. 历史记录搜索、文件处理等
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
     * @param contactId 联系人 ID
     * @param limit 加载数量限制
     * @param offset 偏移量
     */
    // 修复：添加 requestId 参数
    QString loadHistoryMessages(const QString& contactId, int limit = 50, int offset = 0);
    /**
     * @brief 搜索消息
     * @param keyword 搜索关键词
     */
    void searchMessages(const QString& keyword);

    /**
     * @brief 更新消息状态
     * @param messageId 消息 ID
     * @param status 状态（如：pending, sent, delivered, read）
     */
    void updateMessageStatus(const QString& messageId, const QString& status);

    /**
     * @brief 删除消息
     * @param messageId 消息 ID
     */
    void deleteMessage(const QString& messageId);

    /**
     * @brief 加载联系人列表
     */
    void loadContactList();

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
     * @brief 处理文件（压缩、转码等）
     * @param filePath 文件路径
     * @param options 处理选项
     */
    void processFile(const QString& filePath, const QJsonObject& options);

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
     * @param messages 消息列表（JSON 数组）
     * @param contactId 联系人 ID
     */
     // 修复：信号中包含 requestId
    void historyMessagesLoaded(const QString& requestId, const QJsonArray& messages, const QString& contactId);

    /**
     * @brief 消息搜索完成
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

private:
    QString m_dbPath;
    bool m_dbInitialized;
};

#endif // DBLOGIC_WORKER_H
