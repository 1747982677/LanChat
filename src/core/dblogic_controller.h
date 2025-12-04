#ifndef DBLOGIC_CONTROLLER_H
#define DBLOGIC_CONTROLLER_H

#include "core/base_controller.h"
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief ���ݿ���ҵ���߼�������
 * 
 * ���������̣߳�����
 * 1. ���� DbLogicWorker ����������
 * 2. �ṩ���ݿ�������ⲿ�ӿ�
 * 3. ת�������¼����ϲ�
 */
class DbLogicController : public BaseController
{
    Q_OBJECT

public:
    explicit DbLogicController(QObject* parent = nullptr);
    ~DbLogicController() override;

    bool initialize() override;

    /**
     * @brief ��ȡ����
     */
    static DbLogicController& instance();

    /**
     * @brief 检查数据库是否已初始化
     * @return 是否已初始化
     */
    bool isDatabaseInitialized() const;

public slots:
    /**
     * @brief ��ʼ�����ݿ�
     */
    void initializeDatabase(const QString& dbPath);

    /**
     * @brief ������Ϣ
     */
    void saveMessage(const QJsonObject& message);
    
    //  �޸������� requestId��UI ����������֤
    QString loadHistoryMessages(const QString& contactId, int limit = 50, int offset = 0);
    
    /**
     * @brief ������Ϣ
     */
    void searchMessages(const QString& keyword);

    /**
     * @brief ������Ϣ״̬
     */
    void updateMessageStatus(const QString& messageId, const QString& status);

    /**
     * @brief ɾ����Ϣ
     */
    void deleteMessage(const QString& messageId);

    /**
     * @brief ������ϵ���б�
     */
    void loadContactList();

    /**
     * @brief ������ϵ��
     */
    void addContact(const QJsonObject& contactInfo);

    /**
     * @brief ������ϵ��
     */
    void updateContact(const QString& contactId, const QJsonObject& contactInfo);

    /**
     * @brief �����ļ�
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

signals:
    // ���͸� Worker ���ź�
    void requestInitializeDatabase(const QString& dbPath);
    void requestSaveMessage(const QJsonObject& message);
    
    //  �޸��������źŰ��� requestId
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

    // �� Worker ���յ��źţ�ת����
    void databaseInitialized(bool success);
    void messageSaved(bool success, const QString& messageId);
    
    //  �޸�����Ӧ�źŰ��� requestId
    void historyMessagesLoaded(const QString& requestId, const QJsonArray& messages, const QString& contactId);
    
    void searchResultsReady(const QJsonArray& results);
    void messageStatusUpdated(bool success, const QString& messageId);
    void contactListLoaded(const QJsonArray& contacts);
    void contactOperationCompleted(bool success, const QString& operation);
    void fileProcessed(bool success, const QString& filePath, const QString& resultPath);
    void userSearchResult(const QJsonObject& userInfo, bool found);
    void userRegistered(bool success, const QString& userId, const QString& errorMessage);
    void passwordVerified(bool success, const QString& userId, const QString& errorMessage);

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
