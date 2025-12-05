#ifndef ADD_FRIEND_DIALOG_H
#define ADD_FRIEND_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "common/types.h"
#include "ui/main_window/main_window.h"

/**
 * @brief 添加朋友弹窗
 * 功能：根据账号（邮箱）搜索用户并发送好友请求
 */
class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget* parent = nullptr);
    ~AddFriendDialog();

private slots:
    // 搜索按钮点击
    void onSearchButtonClicked();
    
    // 搜索结果返回
    void onSearchResultReady(const LanChat::UserInfo& userInfo);
    
    // 搜索失败
    void onSearchFailed(const QString& errorMessage);
    
    // 发送好友请求
    void onSendFriendRequest();

private:
    // 初始化UI
    void setupUi();
    
    // 设置样式
    void setupStyle();
    
    // 连接信号槽
    void connectSignals();
    
    // 显示搜索结果
    void showSearchResult(const LanChat::UserInfo& userInfo);
    
    // 隐藏搜索结果
    void hideSearchResult();
    
    // 显示错误提示
    void showError(const QString& message);
    
    // 隐藏错误提示
    void hideError();

private:
    // UI组件
    QWidget* m_headerWidget;          // 标题栏
    QLabel* m_titleLabel;            // 标题"添加朋友"
    QPushButton* m_closeButton;      // 关闭按钮
    
    QWidget* m_searchWidget;          // 搜索区域
    QLineEdit* m_searchEdit;          // 搜索输入框
    QPushButton* m_searchButton;      // 搜索按钮
    
    QWidget* m_resultWidget;         // 结果区域
    QLabel* m_resultAvatarLabel;      // 结果头像
    QLabel* m_resultNicknameLabel;    // 结果昵称
    QLabel* m_resultAccountLabel;     // 结果账号
    QPushButton* m_addButton;         // 添加按钮
    
    QLabel* m_errorLabel;             // 错误提示标签
    
    QVBoxLayout* m_mainLayout;       // 主布局
    
    // 当前搜索结果
    LanChat::UserInfo m_currentUser;
    bool m_hasResult;
};

#endif // ADD_FRIEND_DIALOG_H

