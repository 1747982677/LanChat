#include "dblogic_controller.h"
#include "dblogic_worker.h"
#include <QDebug>
#include <QUuid>

DbLogicController* DbLogicController::s_instance = nullptr;

DbLogicController::DbLogicController(QObject* parent)
    : BaseController(parent)
{
}

DbLogicController::~DbLogicController()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

DbLogicController& DbLogicController::instance()
{
    if (!s_instance) {
        s_instance = new DbLogicController();
    }
    return *s_instance;
}

bool DbLogicController::initialize()
{
    QObject* worker = createWorker();
    if (!worker) {
        emit errorOccurred("Failed to create DbLogicWorker");
        return false;
    }

    setupWorkerThread(worker);
    connectSignals();
    QMetaObject::invokeMethod(worker, "initialize", Qt::QueuedConnection);

    qDebug() << "DbLogicController initialized";
    emit initialized();
    return true;
}

QObject* DbLogicController::createWorker()
{
    return new DbLogicWorker();
}

void DbLogicController::connectSignals()
{
    if (!m_worker) {
        return;
    }

    DbLogicWorker* worker = qobject_cast<DbLogicWorker*>(m_worker);
    if (!worker) {
        return;
    }

    // Controller -> Worker 信号
    connect(this, &DbLogicController::requestInitializeDatabase,
            worker, &DbLogicWorker::initializeDatabase);
    connect(this, &DbLogicController::requestSaveMessage,
            worker, &DbLogicWorker::saveMessage);
    connect(this, &DbLogicController::requestLoadHistoryMessages,
            worker, &DbLogicWorker::loadHistoryMessages);
    connect(this, &DbLogicController::requestSearchMessages,
            worker, &DbLogicWorker::searchMessages);
    connect(this, &DbLogicController::requestUpdateMessageStatus,
            worker, &DbLogicWorker::updateMessageStatus);
    connect(this, &DbLogicController::requestDeleteMessage,
            worker, &DbLogicWorker::deleteMessage);
    connect(this, &DbLogicController::requestLoadContactList,
            worker, &DbLogicWorker::loadContactList);
    connect(this, &DbLogicController::requestAddContact,
            worker, &DbLogicWorker::addContact);
    connect(this, &DbLogicController::requestUpdateContact,
            worker, &DbLogicWorker::updateContact);
    connect(this, &DbLogicController::requestProcessFile,
            worker, &DbLogicWorker::processFile);

    // Worker -> Controller 信号（转发）
    connect(worker, &DbLogicWorker::databaseInitialized,
            this, &DbLogicController::databaseInitialized);
    connect(worker, &DbLogicWorker::messageSaved,
            this, &DbLogicController::messageSaved);
    connect(worker, &DbLogicWorker::historyMessagesLoaded,
            this, &DbLogicController::historyMessagesLoaded);
    connect(worker, &DbLogicWorker::searchResultsReady,
            this, &DbLogicController::searchResultsReady);
    connect(worker, &DbLogicWorker::messageStatusUpdated,
            this, &DbLogicController::messageStatusUpdated);
    connect(worker, &DbLogicWorker::contactListLoaded,
            this, &DbLogicController::contactListLoaded);
    connect(worker, &DbLogicWorker::contactOperationCompleted,
            this, &DbLogicController::contactOperationCompleted);
    connect(worker, &DbLogicWorker::fileProcessed,
            this, &DbLogicController::fileProcessed);
    connect(worker, &DbLogicWorker::errorOccurred,
            this, &DbLogicController::errorOccurred);
}

void DbLogicController::initializeDatabase(const QString& dbPath)
{
    qDebug() << "DbLogicController: Request initialize database:" << dbPath;
    emit requestInitializeDatabase(dbPath);
}

void DbLogicController::saveMessage(const QJsonObject& message)
{
    emit requestSaveMessage(message);
}

// 修复：生成并返回 requestId
QString DbLogicController::loadHistoryMessages(const QString& contactId, int limit, int offset)
{
    QString requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "DbLogicController: Request load history for" << contactId << "requestId:" << requestId;
    emit requestLoadHistoryMessages(requestId, contactId, limit, offset);
    return requestId;
}

void DbLogicController::searchMessages(const QString& keyword)
{
    qDebug() << "DbLogicController: Request search messages:" << keyword;
    emit requestSearchMessages(keyword);
}

void DbLogicController::updateMessageStatus(const QString& messageId, const QString& status)
{
    emit requestUpdateMessageStatus(messageId, status);
}

void DbLogicController::deleteMessage(const QString& messageId)
{
    emit requestDeleteMessage(messageId);
}

void DbLogicController::loadContactList()
{
    qDebug() << "DbLogicController: Request load contact list";
    emit requestLoadContactList();
}

void DbLogicController::addContact(const QJsonObject& contactInfo)
{
    emit requestAddContact(contactInfo);
}

void DbLogicController::updateContact(const QString& contactId, const QJsonObject& contactInfo)
{
    emit requestUpdateContact(contactId, contactInfo);
}

void DbLogicController::processFile(const QString& filePath, const QJsonObject& options)
{
    qDebug() << "DbLogicController: Request process file:" << filePath;
    emit requestProcessFile(filePath, options);
}
