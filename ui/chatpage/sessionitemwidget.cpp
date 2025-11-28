#include "sessionitemwidget.h"
#include "pixmaputils.h"
#include <QBitmap>
#include <QPainter>
#include <QPainterPath>

SessionItemWidget::SessionItemWidget(QWidget *parent) : QWidget(parent)
{
    // 头像设置
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(45, 45);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setStyleSheet("background-color: transparent;");

    // 文本区域 (名字 + 消息)
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    m_msgLabel = new QLabel(this);
    m_msgLabel->setStyleSheet("font-size: 12px; color: #888;");

    // 垂直布局放名字和消息
    QVBoxLayout *textVLayout = new QVBoxLayout;
    textVLayout->setContentsMargins(0, 5, 0, 5); // 上下留点空隙
    textVLayout->setSpacing(2); // 名字和消息之间的间距
    textVLayout->addWidget(m_titleLabel);
    textVLayout->addWidget(m_msgLabel);
    textVLayout->addStretch(); // 底部弹簧

    // 时间标签
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet("font-size: 11px; color: #aaa;");
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    // 主水平布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    mainLayout->addWidget(m_avatarLabel);       // 左：头像
    mainLayout->addLayout(textVLayout, 1);      // 中：文本区域 (伸缩因子1，占据剩余空间)
    mainLayout->addWidget(m_timeLabel, 0, Qt::AlignTop);

    // 让子控件不接收鼠标事件
    m_avatarLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_msgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_timeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

}

void SessionItemWidget::setAvatar(const QString &path)
{
    m_avatarLabel->setPixmap(PixmapUtils::clipToCircle(path, 45));

}

void SessionItemWidget::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void SessionItemWidget::setMessage(const QString &message)
{
    // 如果消息太长，进行省略处理 (需要根据宽度计算，这里简单截断示意)
    QString finalMsg = message;
    if (finalMsg.length() > 20) {
        finalMsg = finalMsg.left(18) + "...";
    }
    m_msgLabel->setText(finalMsg);
}

void SessionItemWidget::setTime(const QString &time)
{
    m_timeLabel->setText(time);
}
