#include "add_friend_dialog.h"
#include "core/app_context.h"
#include "core/dblogic_controller.h"
#include "service/auth_service.h"
#include "utils/logger.h"
#include <QIcon>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPixmap>

AddFriendDialog::AddFriendDialog(QWidget* parent)
    : QDialog(parent)
    , m_hasResult(false)
{
    // 使用标准窗口标志，允许拖动和调整大小
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    
    // 设置窗口大小和最小/最大尺寸
    resize(400, 500);
    setMinimumSize(350, 400);
    setMaximumSize(800, 1000);
    
    setupUi();
    setupStyle();
    connectSignals();
    
    hideSearchResult();
    hideError();
}

AddFriendDialog::~AddFriendDialog()
{
}

void AddFriendDialog::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // ===================== 标题栏 =====================
    m_headerWidget = new QWidget(this);
    m_headerWidget->setFixedHeight(50);
    QHBoxLayout* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_titleLabel = new QLabel("添加朋友", m_headerWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    
    headerLayout->addWidget(m_titleLabel);
    
    m_mainLayout->addWidget(m_headerWidget);
    
    // ===================== 搜索区域 =====================
    m_searchWidget = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout(m_searchWidget);
    searchLayout->setContentsMargins(20, 20, 20, 20);
    searchLayout->setSpacing(10);
    
    m_searchEdit = new QLineEdit(m_searchWidget);
    m_searchEdit->setPlaceholderText("搜索好友的邮箱");
    m_searchEdit->setFixedHeight(40);
    
    m_searchButton = new QPushButton("搜索", m_searchWidget);
    m_searchButton->setFixedSize(80, 40);
    m_searchButton->setStyleSheet("QPushButton { background-color: #07c160; color: white; border: none; border-radius: 4px; font-size: 14px; }"
                                  "QPushButton:hover { background-color: #06ad56; }"
                                  "QPushButton:pressed { background-color: #059648; }");
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    
    m_mainLayout->addWidget(m_searchWidget);
    
    // ===================== 错误提示 =====================
    m_errorLabel = new QLabel(this);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setStyleSheet("color: #ff4444; padding: 10px;");
    m_errorLabel->hide();
    m_mainLayout->addWidget(m_errorLabel);
    
    // ===================== 结果区域 =====================
    m_resultWidget = new QWidget(this);
    QVBoxLayout* resultLayout = new QVBoxLayout(m_resultWidget);
    resultLayout->setContentsMargins(20, 20, 20, 20);
    resultLayout->setSpacing(15);
    resultLayout->setAlignment(Qt::AlignTop);
    
    // 头像
    m_resultAvatarLabel = new QLabel(m_resultWidget);
    m_resultAvatarLabel->setFixedSize(80, 80);
    m_resultAvatarLabel->setScaledContents(true);
    m_resultAvatarLabel->setAlignment(Qt::AlignCenter);
    m_resultAvatarLabel->setStyleSheet("border: 1px solid #e0e0e0; border-radius: 4px; background-color: #f5f5f5;");
    resultLayout->addWidget(m_resultAvatarLabel, 0, Qt::AlignHCenter);
    
    // 昵称
    m_resultNicknameLabel = new QLabel(m_resultWidget);
    m_resultNicknameLabel->setAlignment(Qt::AlignCenter);
    m_resultNicknameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");
    resultLayout->addWidget(m_resultNicknameLabel);
    
    // 账号
    m_resultAccountLabel = new QLabel(m_resultWidget);
    m_resultAccountLabel->setAlignment(Qt::AlignCenter);
    m_resultAccountLabel->setStyleSheet("font-size: 14px; color: #999;");
    resultLayout->addWidget(m_resultAccountLabel);
    
    // 添加按钮
    m_addButton = new QPushButton("添加", m_resultWidget);
    m_addButton->setFixedSize(120, 40);
    m_addButton->setStyleSheet("QPushButton { background-color: #07c160; color: white; border: none; border-radius: 4px; font-size: 14px; }"
                                "QPushButton:hover { background-color: #06ad56; }"
                                "QPushButton:pressed { background-color: #059648; }");
    resultLayout->addWidget(m_addButton, 0, Qt::AlignHCenter);
    
    resultLayout->addStretch();
    
    m_mainLayout->addWidget(m_resultWidget);
    
    // ===================== 底部关闭按钮 =====================
    QWidget* bottomWidget = new QWidget(this);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(20, 20, 20, 20);
    bottomLayout->addStretch();
    
    m_closeButton = new QPushButton("Close", bottomWidget);
    m_closeButton->setFixedSize(100, 35);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: #333;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #d0d0d0;"
        "}"
    );
    bottomLayout->addWidget(m_closeButton);
    
    m_mainLayout->addWidget(bottomWidget);
}

void AddFriendDialog::setupStyle()
{
    setStyleSheet(
        "QDialog { background-color: white; }"
        "QLineEdit { border: 2px solid #07c160; border-radius: 4px; padding: 8px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #06ad56; }"
    );
}

void AddFriendDialog::connectSignals()
{
    // 关闭按钮
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 搜索按钮
    connect(m_searchButton, &QPushButton::clicked, this, &AddFriendDialog::onSearchButtonClicked);
    
    // 搜索框回车
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &AddFriendDialog::onSearchButtonClicked);
    
    // 添加按钮
    connect(m_addButton, &QPushButton::clicked, this, &AddFriendDialog::onSendFriendRequest);
    
    // 连接数据库控制器信号
    DbLogicController* dbController = &DbLogicController::instance();
    connect(dbController, &DbLogicController::userSearchResult,
            this, [this](const QJsonObject& userInfo, bool found) {
                if (found) {
                    LanChat::UserInfo info = LanChat::UserInfo::fromJson(userInfo);
                    onSearchResultReady(info);
                } else {
                    onSearchFailed("未找到该用户");
                }
            });
    
    // 连接好友请求发送结果信号
    connect(dbController, &DbLogicController::friendRequestSent,
            this, [this](bool success, const QString& requestId, const QString& errorMessage) {
                if (success) {
                    QMessageBox::information(this, "成功", "好友请求已发送");
                    Logger::getInstance().log("好友请求发送成功，requestId: " + requestId);
                    accept();  // 发送成功后关闭对话框
                } else {
                    showError(errorMessage.isEmpty() ? "发送好友请求失败" : errorMessage);
                    m_addButton->setEnabled(true);  // 重新启用按钮
                    m_addButton->setText("添加");   // 恢复按钮文字
                }
            });
}

void AddFriendDialog::onSearchButtonClicked()
{
    QString account = m_searchEdit->text().trimmed();
    
    if (account.isEmpty()) {
        showError("请输入要搜索的账号");
        return;
    }
    
    // 验证邮箱格式
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(account).hasMatch()) {
        showError("请输入有效的邮箱地址");
        return;
    }
    
    hideError();
    hideSearchResult();
    
    // 显示搜索中状态
    m_searchButton->setText("搜索中...");
    m_searchButton->setEnabled(false);
    
    // 调用搜索
    DbLogicController* dbController = &DbLogicController::instance();
    dbController->searchUserByAccount(account);
}

void AddFriendDialog::onSearchResultReady(const LanChat::UserInfo& userInfo)
{
    m_searchButton->setText("搜索");
    m_searchButton->setEnabled(true);
    
    if (userInfo.userId.isEmpty()) {
        showError("未找到该用户");
        return;
    }
    
    m_currentUser = userInfo;
    m_hasResult = true;
    showSearchResult(userInfo);
}

void AddFriendDialog::onSearchFailed(const QString& errorMessage)
{
    m_searchButton->setText("搜索");
    m_searchButton->setEnabled(true);
    showError(errorMessage);
}

void AddFriendDialog::showSearchResult(const LanChat::UserInfo& userInfo)
{
    // 设置头像
    QString avatarPath = userInfo.avatarPath;
    
    // 如果头像路径为空，使用默认头像
    if (avatarPath.isEmpty()) {
        avatarPath = ":/lanchat/bubu.jpg";
    }
    
    QPixmap pixmap(avatarPath);
    if (!pixmap.isNull()) {
        m_resultAvatarLabel->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // 如果加载失败（可能是路径错误），尝试加载默认头像
        QPixmap defaultPixmap(":/lanchat/bubu.jpg");
        if (!defaultPixmap.isNull()) {
            m_resultAvatarLabel->setPixmap(defaultPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // 如果默认头像也加载失败，显示文字提示
            m_resultAvatarLabel->setText("头像");
        }
    }
    
    // 设置昵称
    m_resultNicknameLabel->setText(userInfo.nickname.isEmpty() ? "未设置昵称" : userInfo.nickname);
    
    // 设置账号
    m_resultAccountLabel->setText(userInfo.email.isEmpty() ? userInfo.account : userInfo.email);
    
    // 检查是否是自己
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    if (currentUserId == userInfo.userId) {
        m_addButton->setEnabled(false);
        m_addButton->setText("这是你自己");
    } else {
        m_addButton->setEnabled(true);
        m_addButton->setText("添加");
    }
    
    m_resultWidget->show();
}

void AddFriendDialog::hideSearchResult()
{
    m_resultWidget->hide();
    m_hasResult = false;
}

void AddFriendDialog::showError(const QString& message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void AddFriendDialog::hideError()
{
    m_errorLabel->hide();
}

void AddFriendDialog::onSendFriendRequest()
{
    if (!m_hasResult || m_currentUser.userId.isEmpty()) {
        showError("请先搜索用户");
        return;
    }
    
    // 获取当前登录用户信息
    AuthService& authService = AuthService::getInstance();
    QString currentUserId = authService.getCurrentUserId();
    
    if (currentUserId.isEmpty()) {
        showError("未登录，请先登录");
        return;
    }
    
    // 不能给自己发送好友请求
    if (currentUserId == m_currentUser.userId) {
        showError("不能给自己发送好友请求");
        return;
    }
    
    // 禁用添加按钮，防止重复点击
    m_addButton->setEnabled(false);
    m_addButton->setText("发送中...");
    
    // 获取当前用户信息（用于好友请求）
    // 从 MainWindow 获取当前用户信息（昵称、头像等）
    MainWindow* mainWindow = MainWindow::instance();
    QString senderAccount = authService.getSavedAccount();  // 从保存的账号获取
    QString senderNickname = mainWindow->m_currentUser.nickname.isEmpty() ? 
                            "用户" : mainWindow->m_currentUser.nickname;
    QString avatarPath = mainWindow->m_currentUser.avatarPath;
    
    // 调用数据库控制器发送好友请求
    DbLogicController* dbController = &DbLogicController::instance();
    dbController->sendFriendRequest(currentUserId, m_currentUser.userId, 
                                   senderAccount, senderNickname, 
                                   avatarPath, QString());  // 验证消息暂时为空
}

