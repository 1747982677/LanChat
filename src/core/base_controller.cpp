#include "base_controller.h"
#include <QDebug>

BaseController::BaseController(QObject* parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_isRunning(false)
{
}

BaseController::~BaseController()
{
    stop();
}

void BaseController::start()
{
    if (m_isRunning) {
        qDebug() << "Controller already running";
        return;
    }

    if (!m_workerThread) {
        qWarning() << "Worker thread not initialized";
        return;
    }

    m_workerThread->start();
    m_isRunning = true;
    qDebug() << "Controller started successfully";
}

void BaseController::stop()
{
    if (!m_isRunning) {
        return;
    }

    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }

    if (m_worker) {
        m_worker->deleteLater();
        m_worker = nullptr;
    }

    m_isRunning = false;
    qDebug() << "Controller stopped";
}

bool BaseController::isRunning() const
{
    return m_isRunning;
}

void BaseController::setupWorkerThread(QObject* worker)
{
    if (!worker) {
        qWarning() << "Worker is null";
        return;
    }

    m_worker = worker;
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    // 线程结束时清理 Worker
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
}
