// ContactItem.cpp
/*
 * 页面名称：联系人列表组件
 * 功能：显示每个单独的联系人
 * 用法：在ContactList集成
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
    QPixmap pix(avatarPath);
    if (!pix.isNull()) m_avatarLabel->setPixmap(pix);
    layout->addWidget(m_avatarLabel);

    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(m_nameLabel);

    // 未读角标，初始隐藏
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
