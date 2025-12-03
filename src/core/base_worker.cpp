#include "base_worker.h"

BaseWorker::BaseWorker(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

BaseWorker::~BaseWorker()
{
    if (m_initialized) {
        cleanup();
    }
}
