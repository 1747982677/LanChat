#ifndef DBLOGIC_WORKER_H
#define DBLOGIC_WORKER_H

#include "core/base_worker.h"
#include <QString>
#include <QJsonObject>

/**
 * @brief ���ݿ���ҵ���߼� Worker
 * 
 * �����ڶ������߼��߳��У�����
 * 1. ���ݿ����ɾ�Ĳ����
 * 2. ��ʱ��ҵ���߼�����
 * 3. ���ݽ�����ת��
 * 4. ��ʷ��¼�������ļ�������
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
     * @brief ��ʼ�����ݿ�
     * @param dbPath ���ݿ�·��
     */
    void initializeDatabase(const QString& dbPath);

    /**
     * @brief ������Ϣ�����ݿ�
     * @param message ��Ϣ����
     */
    void saveMessage(const QJsonObject& message);

    /**
     * @brief ������ʷ��Ϣ
     * @param contactId ��ϵ�� ID
     * @param limit ������������
     * @param offset ƫ����
     */
    // �޸������� requestId ����
    void loadHistoryMessages(const QString& requestId, const QString& contactId, int limit, int offset);
    /**
     * @brief ������Ϣ
     * @param keyword �����ؼ���
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
     * @brief ������Ϣ״̬
     * @param messageId ��Ϣ ID
     * @param status ״̬���磺pending, sent, delivered, read��
     */
    void updateMessageStatus(const QString& messageId, const QString& status);

    /**
     * @brief ɾ����Ϣ
     * @param messageId ��Ϣ ID
     */
    void deleteMessage(const QString& messageId);

    /**
     * @brief ������ϵ���б�
     */
    void loadContactList();

    /**
     * @brief ������ϵ��
     * @param contactInfo ��ϵ����Ϣ
     */
    void addContact(const QJsonObject& contactInfo);

    /**
     * @brief ������ϵ����Ϣ
     * @param contactId ��ϵ�� ID
     * @param contactInfo ��ϵ����Ϣ
     */
    void updateContact(const QString& contactId, const QJsonObject& contactInfo);

    /**
     * @brief �����ļ���ѹ����ת��ȣ�
     * @param filePath �ļ�·��
     * @param options ����ѡ��
     */
    void processFile(const QString& filePath, const QJsonObject& options);

signals:
    /**
     * @brief ���ݿ��ʼ�����
     */
    void databaseInitialized(bool success);

    /**
     * @brief ��Ϣ�������
     */
    void messageSaved(bool success, const QString& messageId);

    /**
     * @brief ��ʷ��Ϣ�������
     * @param messages ��Ϣ�б���JSON ���飩
     * @param contactId ��ϵ�� ID
     */
     // �޸����ź��а��� requestId
    void historyMessagesLoaded(const QString& requestId, const QJsonArray& messages, const QString& contactId);

    /**
     * @brief ��Ϣ�������
     * @param results ���������JSON ���飩
     */
    void searchResultsReady(const QJsonArray& results);

    /**
     * @brief ��Ϣ״̬�������
     */
    void messageStatusUpdated(bool success, const QString& messageId);

    /**
     * @brief ��ϵ���б��������
     */
    void contactListLoaded(const QJsonArray& contacts);

    /**
     * @brief ��ϵ�˲������
     */
    void contactOperationCompleted(bool success, const QString& operation);

    /**
     * @brief �ļ��������
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

private:
    QString m_dbPath;
    bool m_dbInitialized;
};

#endif // DBLOGIC_WORKER_H
