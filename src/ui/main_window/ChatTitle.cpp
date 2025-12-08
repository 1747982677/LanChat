#include "ChatTitle.h"
#include <QHBoxLayout>

ChatTitle::ChatTitle(QWidget* parent) : QWidget(parent)
{
    this->setFixedHeight(60);
    this->setStyleSheet("QWidget { background-color: #f5f5f5; border-bottom: 1px solid #e7e7e7; }");

    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #000000; border: none;");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0); // 左侧留出间距
    layout->setSpacing(10);

    layout->addWidget(m_nameLabel);
    layout->addStretch();
}

void ChatTitle::setSessionInfo(const SessionInfo& info)
{
    m_nameLabel->setText(info.username());

}
