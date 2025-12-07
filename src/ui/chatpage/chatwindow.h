#ifndef CHATWINDOW_H
#define CHATWINDOW_H
#include "messagedata.h"
#include "nchatview.h"
#include <QWidget>
#include <QString>
#include <QList>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

    // 切换当前的聊天对象
    void switchChat(const QString &userId, const QString &userName);

private:
    Ui::ChatWindow *ui;

    QString m_currentUserId;
    QString m_currentUserName;
    QList<MessageData> m_allMessages;

    QLabel *m_nameLabel;
    NChatView *m_chatView;
    QTextEdit *m_inputEdit;
    QPushButton *m_sendBtn;
    int m_displayedCount = 0;
    const int PAGE_SIZE = 15; // 每次加载15条

    void loadDataFromDatabase(const QString &userId);
    void renderHistoryBatch();

private slots:
    void onLoadMoreHistory();
    void onSendClicked();
    void onMessageReceived(const QJsonObject &msgJson, const QString &from);
};

#endif // CHATWINDOW_H
