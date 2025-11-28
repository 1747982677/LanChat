#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <QLabel>
#include <QSplitter>
#include <QListWidget>
#include <QStackedWidget>
#include <QMap>
#include <QLineEdit>
#include <QList> // 用于 LRU 顺序记录
#include "chatwindow.h"

namespace Ui {
class ChatPage;
}

enum ChatItemRoles {
    RoleUserId = Qt::UserRole,      // 用来存 ID
    RoleUserName = Qt::UserRole + 1 // 用来存 名字 (偏移+1)
};

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

private slots:
    void onSessionClicked(QListWidgetItem *item);
    void onSearchTextChanged(const QString &text);

private:
    Ui::ChatPage *ui;

    void initMockData();
    void switchChatWindow(const QString &userId, const QString &userName);

    QSplitter *m_splitter;
    QWidget *m_leftWidget;           // 左侧容器
    QLineEdit *m_searchEdit;         // 搜索框
    QListWidget *m_sessionList;      // 会话列表
    ChatWindow *m_chatWindow;
    QWidget *m_emptyWidget;
    QStackedWidget *m_rightStackedWidget;

    // Key: UserId, Value: 对应的窗口指针
    QMap<QString, ChatWindow*> m_chatWindows;
};

#endif // CHATPAGE_H
