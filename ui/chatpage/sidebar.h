#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include "sidebarbutton.h"


namespace Ui {
class SideBar;
}

class SideBar : public QWidget
{
    Q_OBJECT

public:
    explicit SideBar(QWidget *parent = nullptr);
    ~SideBar();
signals:
    // 发送信号告诉主窗口切换到了第几个页面 (0:Chat, 1:Contact, 2:Settings)
    void pageChanged(int index);

private:
    Ui::SideBar *ui;

    QLabel *m_avatar;         // 头像
    SideBarButton *m_btnChat;   // 聊天
    SideBarButton *m_btnContact;// 通讯录
    SideBarButton *m_btnSetting;// 设置
    QButtonGroup *m_btnGroup; // 互斥组管理

    QToolButton* createBtn(const QString &text, const QString &color);
};

#endif // SIDEBAR_H
