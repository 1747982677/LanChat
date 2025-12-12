#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QMap>
#include <QVBoxLayout>
#include <QString>
#include "ui/personinfo/UserProfile.h"  // 确保包含 UserProfile 头文件
#include "ui/personinfo/UserEntity.h"   // 确保包含 UserEntity 头文件
#include "SessionList.h" 
#include "ChatWindow.h"
#include "Message.h"


class MainWindow : public QWidget {
    Q_OBJECT
public:
    enum PageType {
        ChatPage=0,//聊天页
        FriendInfo,//联系人资料页
        SettingsPage,//设置页
        NewFriendPage,//新朋友页



        GroupPage
    };

    static MainWindow* instance();   // 单例
    void requestQueryUser();
	QString userid;//用户ID供外部访问
    void setRightPages(PageType type);//从外部设置页签

    // 打开聊天页，并切换到指定用户的会话
    void openChatPage(const QString& userId, const QString& displayName);
    
    // 更新用户状态显示（在线/离线）
    void updateUserStatus(bool isOnline);
    
	UserEntity m_currentUser; // 当前用户实体

private slots:
    // 处理发送消息
    void onSendMessage(const QString& targetUid, const UiMessage& msg);
    
    // 加载好友申请列表
    void loadFriendRequests();

private:
    void updateUserReady(const bool& glag);
    UserProfile m_userProfile;
	void queryUserReady(const UserEntity& localUser);
    void showProfileViewDialog();
    bool eventFilter(QObject* watched, QEvent* event) override;  // 使用 override 关键字
    explicit MainWindow(QWidget* parent = nullptr); // 构造函数私有化
    ~MainWindow();

    static MainWindow* m_instance; 
    void setupUi();

    // 左侧导航
    QWidget* m_tabBar;
    QLabel* m_avatarLabel;
    QLabel* m_statusLabel;  // 状态标签（在线/离线）
    QButtonGroup* m_sideButtonGroup;
    void setupLeftNav();

    // 中间页面
    QStackedWidget* m_contactPages;
    QLineEdit* m_searchEdit;
    QWidget* m_middleWidget;
    SessionList* m_sessionList;
    void setupMiddleColumn();


    // 右侧页面
    QStackedWidget* m_pages;
    QMap<PageType, QWidget*> m_pageMap;//枚举类型映射


    //页面List
    ChatWindow* m_chatPage;
    QWidget* m_friendInfoPage;
    QWidget* m_settingsPage;
    QWidget* m_newFriendPage;
    QVBoxLayout* m_friendRequestLayout;  // 好友申请列表布局

    void setupPages();
    
    // 好友申请相关
    void onFriendRequestsLoaded(const QJsonArray& requests);
    void onFriendRequestAccepted(bool success, const QString& errorMessage);
    void onAcceptButtonClicked(const QString& requestId, const QString& senderId);

    //页面映射
    void addRightPage(PageType type, QWidget* page);
    // 弹出设置弹窗
    void openSettingsDialog();
};
