#include "nchatmessageitem.h"
#include "pixmaputils.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

NChatMessageItem::NChatMessageItem(MsgRole role, const QString &text, const QString &avatarPath, QWidget *parent)
    : QWidget(parent), m_role(role), m_text(text), m_avatarPath(avatarPath)
{
    initUi();
}

void NChatMessageItem::initUi()
{
    // 整体布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10); // 头像和气泡的间距

    // 初始化头像
    m_lblAvatar = new QLabel(this);
    m_lblAvatar->setFixedSize(40, 40);
    m_lblAvatar->setPixmap(PixmapUtils::clipToCircle(m_avatarPath, 40));
    m_lblAvatar->setScaledContents(true);
    m_lblAvatar->setContentsMargins(0, 2, 0, 0);

    // 初始化气泡 (使用 QLabel 显示文本)
    m_lblBubble = new QLabel(this);
    m_lblBubble->setText(m_text);
    m_lblBubble->setFont(QFont("Microsoft YaHei", 10)); // 微软雅黑
    m_lblBubble->setWordWrap(true); // 自动换行
    m_lblBubble->setMaximumWidth(400); // 气泡最大宽度，防止占满屏幕
    m_lblBubble->setContentsMargins(12, 8, 12, 8); // 气泡内边距 (文字离边框的距离)
    m_lblBubble->setTextInteractionFlags(Qt::TextSelectableByMouse); // 允许复制文字

    // 根据角色设置样式和布局方向
    if (m_role == MsgRole::Self) {
        // --- 我发的：右对齐 ---

        // 气泡样式
        m_lblBubble->setStyleSheet(
            "QLabel {"
            "   background-color: #ABF7D5;"
            "   color: #3B3B3B;"
            "   border-radius: 6px;"
            "   padding: 5px;"
            "}"
            );

        mainLayout->addStretch();           // 弹簧在左边，把内容顶到右边
        mainLayout->addWidget(m_lblBubble, 0, Qt::AlignTop); // 气泡在前
        mainLayout->addWidget(m_lblAvatar, 0, Qt::AlignTop); // 头像在后

    } else {
        // --- 别人发的：左对齐 ---

        // 气泡样式：白色背景
        m_lblBubble->setStyleSheet(
            "QLabel {"
            "   background-color: #FFFFFF;"
            "   color: black;"
            "   border-radius: 6px;"
            "   padding: 5px;"
            "}"
            );

        mainLayout->addWidget(m_lblAvatar, 0, Qt::AlignTop); // 头像在前
        mainLayout->addWidget(m_lblBubble, 0, Qt::AlignTop); // 气泡在后
        mainLayout->addStretch();           // 弹簧在右边，把内容顶到左边
    }
}

void NChatMessageItem::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int maxBubbleWidth = this->width() * 0.7;
    if (maxBubbleWidth < 240) maxBubbleWidth = 240;
    m_lblBubble->setMaximumWidth(maxBubbleWidth);
    m_lblBubble->adjustSize();
}
