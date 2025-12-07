#include "app_context.h"
#include <QDebug>
#include <QDateTime>

AppContext* AppContext::s_instance = nullptr;

AppContext::AppContext(QObject* parent)
    : QObject(parent)
    , m_networkController(nullptr)
    , m_dbLogicController(nullptr)
    , m_initialized(false)
{
}

AppContext::~AppContext()
{
    stopAll();
    
    if (m_networkController) {
        delete m_networkController;
        m_networkController = nullptr;
    }
    
    if (m_dbLogicController) {
        delete m_dbLogicController;
        m_dbLogicController = nullptr;
    }
}

AppContext& AppContext::instance()
{
    if (!s_instance) {
        s_instance = new AppContext();
    }
    return *s_instance;
}
//初始化
bool AppContext::initialize()
{
    if (m_initialized) {
        qDebug() << "AppContext already initialized";
        return true;
    }

    qDebug() << "Initializing AppContext...";

    m_networkController = &NetworkController::instance();
    if (!m_networkController->initialize()) {
        qWarning() << "Failed to initialize NetworkController";
        emit applicationError("Failed to initialize NetworkController");
        return false;
    }

    m_dbLogicController = &DbLogicController::instance();
    if (!m_dbLogicController->initialize()) {
        qWarning() << "Failed to initialize DbLogicController";
        emit applicationError("Failed to initialize DbLogicController");
        return false;
    }

    connectControllers();

    m_initialized = true;
    qDebug() << "AppContext initialized successfully";
    emit applicationInitialized();
    
    return true;
}

void AppContext::startAll()
{
    if (!m_initialized) {
        qWarning() << "Cannot start: AppContext not initialized";
        return;
    }

    qDebug() << "Starting all controllers...";

    if (m_networkController) {
        m_networkController->start();
    }

    if (m_dbLogicController) {
        m_dbLogicController->start();
    }

    qDebug() << "All controllers started";
}

void AppContext::stopAll()
{
    qDebug() << "Stopping all controllers...";

    if (m_networkController) {
        m_networkController->stop();
    }

    if (m_dbLogicController) {
        m_dbLogicController->stop();
    }

    qDebug() << "All controllers stopped";
}

NetworkController* AppContext::networkController()
{
    return m_networkController;
}

DbLogicController* AppContext::dbLogicController()
{
    return m_dbLogicController;
}

void AppContext::setDatabasePath(const QString& path)
{
    m_databasePath = path;
}

QString AppContext::databasePath() const
{
    return m_databasePath;
}

void AppContext::connectControllers()
{
    if (m_networkController && m_dbLogicController) {
        //  1���յ�������Ϣ�󱣴浽���ݿ�
        connect(m_networkController, &NetworkController::messageReceived,
                this, [this](const QJsonObject& message, const QString& from) {
                    qDebug() << "AppContext: Network message received, saving to DB";
                    
                    QJsonObject fullMessage = message;
                    fullMessage["from"] = from;
                    fullMessage["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                    fullMessage["status"] = "received";  // �յ�����Ϣ״̬Ϊ received
                    
                    m_dbLogicController->saveMessage(fullMessage);
                });

        //  2����Ϣ���ͳɹ� -> �������ݿ�״̬
        connect(m_networkController, &NetworkController::messageSendSuccess,
                this, [this](const QString& messageId) {
                    qDebug() << "AppContext: Message send success, updating status:" << messageId;
                    m_dbLogicController->updateMessageStatus(messageId, "sent");
                });

        //  3����Ϣ����ʧ�� -> �������ݿ�״̬Ϊʧ��
        connect(m_networkController, &NetworkController::messageSendFailed,
                this, [this](const QString& messageId, const QString& reason) {
                    qWarning() << "AppContext: Message send failed:" << messageId << reason;
                    m_dbLogicController->updateMessageStatus(messageId, "failed");
                    
                    // ����֪ͨ UI ��ʾ���԰�ť
                    emit applicationError(QString("Message send failed: %1").arg(reason));
                });

        // �����ź�ת��
        connect(m_networkController, &NetworkController::errorOccurred,
                this, &AppContext::applicationError);
        connect(m_dbLogicController, &DbLogicController::errorOccurred,
                this, &AppContext::applicationError);
    }
}
