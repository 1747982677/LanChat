#include "NChatmessageItem.h"
#include "PixmapUtils.h"
#include <QPainter>
#include <QDebug>

NChatMessageItem::NChatMessageItem(bool isSelf, const QString& text, const QString& avatarPath, QWidget* parent)
    : QWidget(parent), m_isSelf(isSelf), m_text(text), m_avatarPath(avatarPath)
{
    initUi();
}

void NChatMessageItem::initUi()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    m_lblAvatar = new QLabel(this);
    m_lblAvatar->setFixedSize(40, 40);
    QPixmap avatarPix = PixmapUtils::clipToCircle(m_avatarPath, 40);
    if (avatarPix.isNull()) {
        m_lblAvatar->setStyleSheet("background-color: #ccc; border-radius: 20px;"); // 占位
    }
    else {
        m_lblAvatar->setPixmap(avatarPix);
    }
    m_lblAvatar->setScaledContents(true);

    m_lblBubble = new QLabel(this);
    m_lblBubble->setText(m_text);
    m_lblBubble->setFont(QFont("Microsoft YaHei", 10));
    m_lblBubble->setWordWrap(true); 
    m_lblBubble->setContentsMargins(12, 8, 12, 8);
    m_lblBubble->setTextInteractionFlags(Qt::TextSelectableByMouse);

    if (m_isSelf) {
        // 自己发的消息：绿色背景，右对齐
        m_lblBubble->setStyleSheet(
            "QLabel { background-color: #ABF7D5; color: #000; border-radius: 6px; padding: 5px; }"
        );
        m_mainLayout->addStretch();
        m_mainLayout->addWidget(m_lblBubble, 0, Qt::AlignTop);
        m_mainLayout->addWidget(m_lblAvatar, 0, Qt::AlignTop);
    }
    else {
        // 对方的消息：白色背景，左对齐
        m_lblBubble->setStyleSheet(
            "QLabel { background-color: #FFFFFF; color: #000; border-radius: 6px; padding: 5px; }"
        );
        m_mainLayout->addWidget(m_lblAvatar, 0, Qt::AlignTop);
        m_mainLayout->addWidget(m_lblBubble, 0, Qt::AlignTop);
        m_mainLayout->addStretch();
    }
}

QSize NChatMessageItem::sizeHint() const
{

    return m_mainLayout->sizeHint();
}

void NChatMessageItem::adjustContent(int width)
{
    this->resize(width, this->height());

    int maxBubbleWidth = width * 0.7; // 70%
    if (maxBubbleWidth < 200) maxBubbleWidth = 200;

    m_lblBubble->setMaximumWidth(maxBubbleWidth);

    m_lblBubble->adjustSize();
}

void NChatMessageItem::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    adjustContent(event->size().width());
}
