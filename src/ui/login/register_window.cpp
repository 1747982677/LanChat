#include "register_window.h"
#include "service/auth_service.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QRegularExpression>

RegisterWindow::RegisterWindow(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_globalHintLabel(nullptr)
    , m_accountEdit(nullptr)
    , m_accountHintLabel(nullptr)
    , m_passwordEdit(nullptr)
    , m_passwordHintLabel(nullptr)
    , m_agreementCheckBox(nullptr)
    , m_registerButton(nullptr)
    , m_returnToLoginLinkLabel(nullptr)
{
    setupUi();
    setupStyle();
    connectSignals();
    
    // 设置窗口属性
    setWindowTitle("LanChat - 注册");
    resize(400, 500);
    
    // 居中显示
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

RegisterWindow::~RegisterWindow()
{
}

void RegisterWindow::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(40, 30, 40, 30);
    
    // 标题（欢迎注册+应用名）
    m_titleLabel = new QLabel("欢迎注册 LanChat", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_mainLayout->addWidget(m_titleLabel);
    
    // 全局提示区域（顶部，初始隐藏）
    m_globalHintLabel = new QLabel(this);
    m_globalHintLabel->setAlignment(Qt::AlignCenter);
    m_globalHintLabel->setWordWrap(true);
    m_globalHintLabel->setMinimumHeight(30);
    m_globalHintLabel->hide();
    m_mainLayout->addWidget(m_globalHintLabel);
    
    m_mainLayout->addStretch();
    
    // 表单布局
    QVBoxLayout* formLayout = new QVBoxLayout();
    formLayout->setSpacing(10);
    
    // 账号输入框（邮箱）
    m_accountEdit = new QLineEdit(this);
    m_accountEdit->setPlaceholderText("请输入邮箱账号");
    m_accountEdit->setMinimumHeight(40);
    formLayout->addWidget(m_accountEdit);
    
    // 账号提示文案（初始隐藏）
    m_accountHintLabel = new QLabel(this);
    m_accountHintLabel->setWordWrap(true);
    m_accountHintLabel->setStyleSheet("color: #dc3545; font-size: 12px; padding: 4px;");
    m_accountHintLabel->hide();
    formLayout->addWidget(m_accountHintLabel);
    
    // 密码输入框
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码（6-16位字母或数字）");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(40);
    formLayout->addWidget(m_passwordEdit);
    
    // 密码提示文案（初始隐藏）
    m_passwordHintLabel = new QLabel(this);
    m_passwordHintLabel->setWordWrap(true);
    m_passwordHintLabel->setStyleSheet("color: #dc3545; font-size: 12px; padding: 4px;");
    m_passwordHintLabel->hide();
    formLayout->addWidget(m_passwordHintLabel);
    
    m_mainLayout->addLayout(formLayout);
    
    // 协议复选框
    m_agreementCheckBox = new QCheckBox("已阅读并同意注册协议", this);
    m_mainLayout->addWidget(m_agreementCheckBox);
    
    // 注册按钮（初始禁用）
    m_registerButton = new QPushButton("注册", this);
    m_registerButton->setMinimumHeight(45);
    m_registerButton->setEnabled(false);  // 初始禁用，需要勾选协议
    m_mainLayout->addWidget(m_registerButton);
    
    m_mainLayout->addStretch();
    
    // 返回登录链接（底部右侧）
    QHBoxLayout* linkLayout = new QHBoxLayout();
    linkLayout->addStretch();
    m_returnToLoginLinkLabel = new QLabel("<a href=\"#\">返回登录</a>", this);
    m_returnToLoginLinkLabel->setOpenExternalLinks(false);
    linkLayout->addWidget(m_returnToLoginLinkLabel);
    m_mainLayout->addLayout(linkLayout);
    
    setLayout(m_mainLayout);
}

void RegisterWindow::setupStyle()
{
    // 设置窗口样式（与登录窗口保持一致）
    setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #0078d4;"
        "}"
        "QPushButton {"
        "    background-color: #0078d4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005a9e;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
        "QLabel#globalHint {"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    font-size: 13px;"
        "}"
        "QLabel#globalHintError {"
        "    background-color: #fff3cd;"
        "    color: #856404;"
        "    border: 1px solid #ffc107;"
        "}"
        "QLabel#globalHintInfo {"
        "    background-color: #d1ecf1;"
        "    color: #0c5460;"
        "    border: 1px solid #bee5eb;"
        "}"
    );
    
    // 设置全局提示标签的对象名
    m_globalHintLabel->setObjectName("globalHint");
}

void RegisterWindow::connectSignals()
{
    // 连接注册按钮点击信号
    connect(m_registerButton, &QPushButton::clicked, 
            this, &RegisterWindow::onRegisterButtonClicked);
    
    // 连接回车键注册
    connect(m_accountEdit, &QLineEdit::returnPressed, 
            this, &RegisterWindow::onRegisterButtonClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, 
            this, &RegisterWindow::onRegisterButtonClicked);
    
    // 连接协议复选框状态变化
    connect(m_agreementCheckBox, &QCheckBox::stateChanged, 
            this, &RegisterWindow::onAgreementCheckBoxStateChanged);
    
    // 连接返回登录链接点击
    connect(m_returnToLoginLinkLabel, &QLabel::linkActivated, 
            this, &RegisterWindow::onReturnToLoginLinkClicked);
    
    // 连接输入框文本变化（实时验证）
    connect(m_accountEdit, &QLineEdit::textChanged, 
            this, &RegisterWindow::onAccountTextChanged);
    connect(m_passwordEdit, &QLineEdit::textChanged, 
            this, &RegisterWindow::onPasswordTextChanged);
    
    // 连接 AuthService 的信号
    AuthService& authService = AuthService::getInstance();
    connect(&authService, &AuthService::registerSucceeded, 
            this, &RegisterWindow::onRegisterSucceeded);
    connect(&authService, &AuthService::registerFailed, 
            this, &RegisterWindow::onRegisterFailed);
}

void RegisterWindow::onRegisterButtonClicked()
{
    QString account = m_accountEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    // 隐藏之前的错误信息
    hideGlobalHint();
    hideAccountHint();
    hidePasswordHint();
    
    // 验证输入
    bool isValid = true;
    
    if (account.isEmpty()) {
        showAccountHint("请输入账号");
        isValid = false;
    } else if (!validateEmail(account)) {
        showAccountHint("请输入有效的邮箱地址");
        isValid = false;
    }
    
    if (password.isEmpty()) {
        showPasswordHint("请输入密码");
        isValid = false;
    } else if (!validatePassword(password)) {
        showPasswordHint("密码格式错误，请使用 6-16 位字母或数字");
        isValid = false;
    }
    
    if (!isValid) {
        return;
    }
    
    // 检查协议复选框
    if (!m_agreementCheckBox->isChecked()) {
        showGlobalHint("请先阅读并同意注册协议", true);
        return;
    }
    
    // 显示"注册中..."提示
    showGlobalHint("注册中...", false);
    
    // 禁用注册按钮（防止重复点击）
    m_registerButton->setEnabled(false);
    m_agreementCheckBox->setEnabled(false);
    
    // 调用 AuthService 进行注册
    AuthService::getInstance().registerAccount(account, password);
}

void RegisterWindow::onRegisterSucceeded(const QString& account)
{
    // 恢复注册按钮状态
    m_registerButton->setEnabled(true);
    m_agreementCheckBox->setEnabled(true);
    
    // 隐藏提示信息
    hideGlobalHint();
    
    // 发出注册成功信号，传递账号和密码（用于自动填充登录界面）
    QString password = m_passwordEdit->text();
    emit registerSucceeded(account, password);
}

void RegisterWindow::onRegisterFailed(const QString& errorMessage, const QString& errorType)
{
    // 恢复注册按钮状态
    m_registerButton->setEnabled(true);
    m_agreementCheckBox->setEnabled(true);
    
    // 根据错误类型显示在不同位置
    if (errorType == "account_exists") {
        // 账号唯一性校验失败：显示在账号提示文案区域
        showAccountHint(errorMessage);
    } else if (errorType == "password_invalid") {
        // 密码格式校验失败：显示在密码提示文案区域
        showPasswordHint(errorMessage);
    } else {
        // 网络连接失败等：显示在全局提示区域
        showGlobalHint(errorMessage, true);
    }
}

void RegisterWindow::onAgreementCheckBoxStateChanged(int state)
{
    // 根据协议复选框状态启用/禁用注册按钮
    bool isChecked = (state == Qt::Checked);
    m_registerButton->setEnabled(isChecked);
}

void RegisterWindow::onReturnToLoginLinkClicked()
{
    // 发出返回登录信号
    emit returnToLogin();
}

void RegisterWindow::onAccountTextChanged(const QString& text)
{
    Q_UNUSED(text);
    // 实时验证：如果输入了内容但格式不对，显示提示
    // 如果输入为空或格式正确，隐藏提示
    if (m_accountEdit->text().isEmpty()) {
        hideAccountHint();
    } else if (!validateEmail(m_accountEdit->text())) {
        // 只在用户输入了内容但格式不对时显示提示
        // 避免在用户刚开始输入时就显示错误
    }
}

void RegisterWindow::onPasswordTextChanged(const QString& text)
{
    Q_UNUSED(text);
    // 实时验证：如果输入了内容但格式不对，显示提示
    if (m_passwordEdit->text().isEmpty()) {
        hidePasswordHint();
    } else if (!validatePassword(m_passwordEdit->text())) {
        // 只在用户输入了内容但格式不对时显示提示
    }
}

bool RegisterWindow::validateEmail(const QString& email) const
{
    // 简单的邮箱格式验证
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QRegularExpressionMatch match = emailRegex.match(email);
    return match.hasMatch();
}

bool RegisterWindow::validatePassword(const QString& password) const
{
    // 验证密码格式：6-16位字母或数字
    if (password.length() < 6 || password.length() > 16) {
        return false;
    }
    
    QRegularExpression passwordRegex(R"(^[a-zA-Z0-9]+$)");
    QRegularExpressionMatch match = passwordRegex.match(password);
    return match.hasMatch();
}

void RegisterWindow::showGlobalHint(const QString& message, bool isError)
{
    m_globalHintLabel->setText(message);
    if (isError) {
        m_globalHintLabel->setObjectName("globalHintError");
    } else {
        m_globalHintLabel->setObjectName("globalHintInfo");
    }
    m_globalHintLabel->setStyleSheet(styleSheet());  // 重新应用样式
    m_globalHintLabel->show();
}

void RegisterWindow::hideGlobalHint()
{
    m_globalHintLabel->hide();
}

void RegisterWindow::showAccountHint(const QString& message)
{
    m_accountHintLabel->setText(message);
    m_accountHintLabel->show();
}

void RegisterWindow::showPasswordHint(const QString& message)
{
    m_passwordHintLabel->setText(message);
    m_passwordHintLabel->show();
}

void RegisterWindow::hideAccountHint()
{
    m_accountHintLabel->hide();
}

void RegisterWindow::hidePasswordHint()
{
    m_passwordHintLabel->hide();
}

