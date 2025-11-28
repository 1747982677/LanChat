// ContactItem.cpp
/*
 * 页面名称：联系人列表组件
 * 功能：显示每个单独的联系人
 * 用法：在ContactList集成
 */
#include "ContactItem.h"
#include <QHBoxLayout>
#include <QPixmap>
//


ContactItem::ContactItem(const QString& name, const QString& avatarPath, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(8);

    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(40, 40);
    m_avatarLabel->setScaledContents(true);

    QPixmap pixmap(avatarPath);
    if (!pixmap.isNull()) {
        m_avatarLabel->setPixmap(pixmap);
    }
    else {
        m_avatarLabel->setStyleSheet("background-color:#666;");
    }

    m_nameLabel = new QLabel(name, this);

    layout->addWidget(m_avatarLabel);
    layout->addWidget(m_nameLabel);
    setLayout(layout);

    // 设置默认和悬停样式
    m_defaultStyle = "background-color: transparent; border-radius: 4px;";
    m_hoverStyle = "background-color: #e0e0e0; border-radius: 4px;";

    setStyleSheet(m_defaultStyle);
}

void ContactItem::enterEvent(QEnterEvent* event)
{
    setStyleSheet(m_hoverStyle);  // 鼠标悬停背景色
    emit hoverEntered(this);
    QWidget::enterEvent(event);
}

void ContactItem::leaveEvent(QEvent* event)
{
    setStyleSheet(m_defaultStyle);  // 恢复默认背景
    emit hoverLeft(this);
    QWidget::leaveEvent(event);
}
