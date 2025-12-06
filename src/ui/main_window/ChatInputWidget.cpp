#include "ChatInputWidget.h"
#include <QDebug>
#include <QMenu>

ChatInputWidget::ChatInputWidget(QWidget* parent) : QWidget(parent)
{
    initUI();
    initStyle();

    // 安装事件过滤器以捕获回车键
    m_textEdit->installEventFilter(this);
}

void ChatInputWidget::initUI()
{

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("请输入消息...");

    m_btnSend = new QPushButton("发送", this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    mainLayout->addWidget(m_textEdit);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch(); // 弹簧
    bottomLayout->addWidget(m_btnSend);

    mainLayout->addLayout(bottomLayout);

    connect(m_btnSend, &QPushButton::clicked, this, &ChatInputWidget::onSendClicked);
}

void ChatInputWidget::initStyle()
{

    this->setStyleSheet("ChatInputWidget { background-color: #F5F5F5; }");

    // 输入框样式：去边框，背景透明或纯白
    m_textEdit->setStyleSheet(
        "QTextEdit {"
        "   border: none;"
        "   background-color: transparent;" // 或者 white
        "   font-size: 14px;"
        "   color: #333333;"
        "}"
    );


    QString btnStyle =
        "QPushButton {"
        "   background-color: #24E0AB;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 5px 15px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #24E0AB; }"
        "QPushButton:pressed { background-color: #24E0AB; }";

    m_btnSend->setStyleSheet(btnStyle);
}

bool ChatInputWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        // 逻辑：按下 Enter 发送，按下 Ctrl+Enter 换行
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                // Ctrl + Enter -> 插入换行符
                m_textEdit->append("");
                return true;
            }
            else {
                // 单独 Enter -> 发送
                onSendClicked();
                return true; // 拦截事件，不让TextEdit处理（防止产生换行）
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChatInputWidget::onSendClicked()
{
    QString text = m_textEdit->toPlainText();
    if (text.isEmpty()) {
        return;
    }

    emit signalSendMessage(text);
    m_textEdit->clear(); // 发送完清空
}

