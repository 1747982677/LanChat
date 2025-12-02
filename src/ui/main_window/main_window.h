#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QMap>  
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
    void setRightPages(PageType type);//从外部设置页签

private:
    explicit MainWindow(QWidget* parent = nullptr); // 构造函数私有化
    ~MainWindow();

    static MainWindow* m_instance; 
    void setupUi();

    // 左侧导航
    QWidget* m_tabBar;
    QLabel* m_avatarLabel;
    QButtonGroup* m_sideButtonGroup;
    void setupLeftNav();

    // 中间页面
    QStackedWidget* m_contactPages;
    QLineEdit* m_searchEdit;
    QWidget* m_middleWidget;
    void setupMiddleColumn();


    // 右侧页面
    QStackedWidget* m_pages;
    QMap<PageType, QWidget*> m_pageMap;//枚举类型映射

    //页面List
    QWidget* m_chatPage;
    QWidget* m_friendInfoPage;
    QWidget* m_settingsPage;
    QWidget* m_newFriendPage;

    void setupPages();

    //页面映射
    void addRightPage(PageType type, QWidget* page);
    // 弹出设置弹窗
    void openSettingsDialog();
};
