// ContactItem.cpp
/*
 * 页面名称：联系人列表项
 * 功能：显示每个联系人的信息
 * 用法：由ContactList调用
 */
#include "ContactItem.h"
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QSizePolicy>

ContactItem::ContactItem(const QString& name, const QString& avatarPath, QWidget* parent)
    : QWidget(parent)
{
    m_defaultStyle = "background: transparent;";
    m_hoverStyle = "background: rgba(0,0,0,0.04);";

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(8);

    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(40, 40);
    m_avatarLabel->setScaledContents(true);
    
    // 加载头像，如果失败则使用默认头像
    QPixmap pix(avatarPath);
    if (!pix.isNull()) {
        m_avatarLabel->setPixmap(pix);
    } else {
        // 如果加载失败（可能是路径无效），使用默认头像
        QPixmap defaultPixmap(":/lanchat/bubu.jpg");
        if (!defaultPixmap.isNull()) {
            m_avatarLabel->setPixmap(defaultPixmap);
        }
    }
    layout->addWidget(m_avatarLabel);

    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(m_nameLabel);
    
    // 状态指示器（小圆点）
    m_statusIndicator = new QLabel(this);
    m_statusIndicator->setFixedSize(8, 8);
    m_statusIndicator->setStyleSheet(
        "QLabel {"
        "   background-color: #999999;"
        "   border-radius: 4px;"
        "}"
    );
    layout->addWidget(m_statusIndicator);

    // 未读消息徽章，初始隐藏
    m_badgeLabel = new QLabel(this);
    m_badgeLabel->setVisible(false);
    m_badgeLabel->setStyleSheet(
        "QLabel { background: red; color: white; border-radius: 10px; padding: 2px 6px; font-size: 12px; }");
    layout->addWidget(m_badgeLabel);

    setStyleSheet(m_defaultStyle);
}

void ContactItem::setUnreadCount(int count)
{
    if (count <= 0) {
        m_badgeLabel->setVisible(false);
    } else {
        m_badgeLabel->setText(count > 99 ? "99+" : QString::number(count));
        m_badgeLabel->setVisible(true);
    }
}

void ContactItem::setUserStatus(bool isOnline)
{
    if (!m_statusIndicator) {
        return;
    }
    
    if (isOnline) {
        // 在线：绿色实心圆点
        m_statusIndicator->setStyleSheet(
            "QLabel {"
            "   background-color: #07c160;"
            "   border-radius: 4px;"
            "}"
        );
    } else {
        // 离线：灰色实心圆点
        m_statusIndicator->setStyleSheet(
            "QLabel {"
            "   background-color: #999999;"
            "   border-radius: 4px;"
            "}"
        );
    }
}

void ContactItem::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    setStyleSheet(m_hoverStyle);
    emit hoverEntered(this);
}

void ContactItem::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    setStyleSheet(m_defaultStyle);
    emit hoverLeft(this);
}

void ContactItem::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    emit clicked(this);
}
