#include "sidebarbutton.h"

SideBarButton::SideBarButton(const QString &text, const QString &normalIconPath,
                            const QString &activeIconPath, QWidget *parent): QToolButton(parent)
{
    setText(text);
    setCheckable(true);        // 允许选中
    setAutoRaise(true);        // 鼠标悬停显示背景
    setToolButtonStyle(Qt::ToolButtonIconOnly); // 只显示图标
    setFixedSize(40, 40);
    setIconSize(QSize(40, 40)); // 设定图标大小，根据需要调整

    // 分别为 "Off" (未选中) 和 "On" (选中) 状态设置图片
    QIcon icon;
    icon.addFile(normalIconPath, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(activeIconPath, QSize(), QIcon::Normal, QIcon::On);
    setIcon(icon);

    setStyleSheet(
        "QToolButton {"
        "    border-radius: 8px;"
        "    border: none;"
        "    padding: 10px;"
        "    background-color: transparent;"
        "}"

        "QToolButton:hover {"
        "    background-color: #e6e6e6;" /* 鼠标悬停背景 */
        "}"
    );
    setFocusPolicy(Qt::NoFocus);


}
