// ProfileViewDialog.cpp - 渐变分割线版
#include "ProfileViewDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFrame>

ProfileViewDialog::ProfileViewDialog(const UserProfile& profile, QWidget* parent)
    : QDialog(parent), m_profile(profile)
    , avatarLabel(nullptr)
    , nicknameLabel(nullptr)
    , emailLabel(nullptr)
    , phoneLabel(nullptr)
    , editButton(nullptr)
{
    setupUI();
    setWindowTitle("个人信息");
    setFixedSize(380, 460);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ProfileViewDialog::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // === 顶部标题栏 ===
    QFrame* headerFrame = new QFrame(this);
    headerFrame->setFixedHeight(40);
    headerFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #4CAF50;"
        "   border-bottom: 1px solid #45a049;"
        "}"
    );

    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(15, 0, 15, 0);

    QLabel* titleLabel = new QLabel("个人资料", headerFrame);
    titleLabel->setFont(QFont("微软雅黑", 12, QFont::Bold));
    titleLabel->setStyleSheet("color: white;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // === 内容区域 ===
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: white;");

    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(8);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    // === 头像区域（居中）===
    QHBoxLayout* avatarContainer = new QHBoxLayout();
    avatarLabel = new QLabel(contentWidget);

    if (!m_profile.avatar.isNull()) {
        avatarLabel->setPixmap(m_profile.avatar.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        avatarLabel->setText("头像");
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setStyleSheet("color: #999; font-size: 13px;");
    }

    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #E0E0E0;"
        "   background-color: #F8F8F8;"
        "   border-radius: 5px;"
        "}"
    );

    avatarContainer->addStretch();
    avatarContainer->addWidget(avatarLabel);
    avatarContainer->addStretch();

    // === 基本信息 ===
    QLabel* userNameLabel = new QLabel(
        m_profile.nickname.isEmpty() ? "未设置昵称" : m_profile.nickname,
        contentWidget
    );
    userNameLabel->setFont(QFont("微软雅黑", 13, QFont::Bold));
    userNameLabel->setStyleSheet("color: #333; margin-top: 8px;");
    userNameLabel->setAlignment(Qt::AlignCenter);

    QLabel* userEmailLabel = new QLabel(
        m_profile.email.isEmpty() ? "邮箱：未设置" : m_profile.email,
        contentWidget
    );
    userEmailLabel->setFont(QFont("微软雅黑", 10));
    userEmailLabel->setStyleSheet("color: #666; margin-bottom: 15px;");
    userEmailLabel->setAlignment(Qt::AlignCenter);

    // === 主分割线（渐变效果）===
    QFrame* mainSeparator = new QFrame(contentWidget);
    mainSeparator->setFixedHeight(1);
    mainSeparator->setStyleSheet(
        "QFrame {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "   stop:0 transparent,"
        "   stop:0.15 #E0E0E0,"
        "   stop:0.85 #E0E0E0,"
        "   stop:1 transparent);"
        "   margin: 5px 10px;"
        "}"
    );

    // === 信息列表 ===
    QFrame* infoFrame = new QFrame(contentWidget);
    infoFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #FAFAFA;"
        "   border: 1px solid #EEE;"
        "   border-radius: 5px;"
        "   margin-top: 8px;"
        "}"
    );

    QVBoxLayout* infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(15, 12, 15, 12);

    // 创建渐变分割线函数
    auto createGradientLine = [](QWidget* parent) -> QFrame* {
        QFrame* line = new QFrame(parent);
        line->setFixedHeight(1);
        line->setStyleSheet(
            "QFrame {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
            "   stop:0 transparent,"
            "   stop:0.2 #E8E8E8,"
            "   stop:0.8 #E8E8E8,"
            "   stop:1 transparent);"
            "   margin: 6px 5px;"
            "}"
        );
        return line;
        };

    // 创建信息行函数
    auto createInfoRow = [&](const QString& title, const QString& value) -> QHBoxLayout* {
        QHBoxLayout* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(10);

        // 标题（右对齐）
        QLabel* titleLabel = new QLabel(title + "：", infoFrame);
        titleLabel->setFont(QFont("微软雅黑", 9));
        titleLabel->setStyleSheet("color: #555; min-width: 60px;");
        titleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // 值（左对齐）
        QLabel* valLabel = new QLabel(value, infoFrame);
        valLabel->setFont(QFont("微软雅黑", 9));
        valLabel->setStyleSheet("color: #333; padding: 4px 0;");
        valLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        valLabel->setWordWrap(true);

        // 存储需要更新的标签
        if (title == "昵称") {
            nicknameLabel = valLabel;
        }
        else if (title == "邮箱") {
            emailLabel = valLabel;
        }
        else if (title == "电话") {
            phoneLabel = valLabel;
        }

        rowLayout->addWidget(titleLabel);
        rowLayout->addWidget(valLabel);
        rowLayout->addStretch();

        return rowLayout;
        };

    // 用户ID
    QString userId = m_profile.userid;//"UID_" + QString::number(qHash(m_profile.nickname + m_profile.email)).mid(2, 6);
    QHBoxLayout* idLayout = new QHBoxLayout();
    QLabel* idTitle = new QLabel("用户ID：", infoFrame);
    idTitle->setFont(QFont("微软雅黑", 9));
    idTitle->setStyleSheet("color: #555; min-width: 70px;");
    idTitle->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel* idValue = new QLabel(userId, infoFrame);
    idValue->setFont(QFont("微软雅黑", 9, QFont::Bold));
    idValue->setStyleSheet("color: #4CAF50; padding: 4px 0;");
    idValue->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    idLayout->addWidget(idTitle);
    idLayout->addWidget(idValue);
    idLayout->addStretch();
    infoLayout->addLayout(idLayout);

    // 渐变分割线1
    infoLayout->addWidget(createGradientLine(infoFrame));

    // 昵称
    infoLayout->addLayout(createInfoRow("昵称",
        m_profile.nickname.isEmpty() ? "未设置" : m_profile.nickname));

    // 渐变分割线2
    infoLayout->addWidget(createGradientLine(infoFrame));

    // 性别
    infoLayout->addLayout(createInfoRow("签名",
        m_profile.signure.isEmpty() ? "未设置" : m_profile.signure));

    // 渐变分割线3
    infoLayout->addWidget(createGradientLine(infoFrame));

    // 邮箱
    infoLayout->addLayout(createInfoRow("邮箱",
        m_profile.email.isEmpty() ? "未设置" : m_profile.email));

    // 渐变分割线4
    infoLayout->addWidget(createGradientLine(infoFrame));

    // 电话
    infoLayout->addLayout(createInfoRow("电话",
        m_profile.phone.isEmpty() ? "未设置" : m_profile.phone));

    // 渐变分割线5
    infoLayout->addWidget(createGradientLine(infoFrame));

    // 密码
    infoLayout->addLayout(createInfoRow("密码", "••••••••"));

    // 添加弹性空间
    infoLayout->addStretch();

    // === 编辑按钮（居中）===
    QHBoxLayout* buttonContainer = new QHBoxLayout();
    editButton = new QPushButton("编辑资料", contentWidget);
    editButton->setFixedSize(120, 36);
    editButton->setFont(QFont("微软雅黑", 10));
    editButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "   stop:0 #5DC560, stop:1 #4CAF50);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "   stop:0 #4CAF50, stop:1 #45a049);"
        "   box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "   stop:0 #45a049, stop:1 #3d8b40);"
        "}"
    );

    buttonContainer->addStretch();
    buttonContainer->addWidget(editButton);
    buttonContainer->addStretch();

    // === 组装内容区域 ===
    contentLayout->addLayout(avatarContainer);
    contentLayout->addWidget(userNameLabel);
    contentLayout->addWidget(userEmailLabel);
    contentLayout->addWidget(mainSeparator);
    contentLayout->addWidget(infoFrame);
    contentLayout->addSpacing(10);
    contentLayout->addLayout(buttonContainer);

    // === 组装主布局 ===
    mainLayout->addWidget(headerFrame);
    mainLayout->addWidget(contentWidget);

    // 连接信号
    connect(editButton, &QPushButton::clicked, this, [this]() {
        emit editRequested();
        });
}

void ProfileViewDialog::updateProfile(const UserProfile& newProfile)
{
    m_profile = newProfile;
    updateDisplay();
}

void ProfileViewDialog::updateDisplay()
{
    // 更新头像
    if (!m_profile.avatar.isNull()) {
        avatarLabel->setPixmap(m_profile.avatar.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        avatarLabel->setStyleSheet(
            "QLabel {"
            "   border: 1px solid #E0E0E0;"
            "   background-color: #F8F8F8;"
            "   border-radius: 5px;"
            "}"
        );
    }

    // 更新文本信息
    if (nicknameLabel) {
        nicknameLabel->setText(m_profile.nickname.isEmpty() ? "未设置" : m_profile.nickname);
    }

    if (emailLabel) {
        emailLabel->setText(m_profile.email.isEmpty() ? "未设置" : m_profile.email);
    }

    if (phoneLabel) {
        phoneLabel->setText(m_profile.phone.isEmpty() ? "未设置" : m_profile.phone);
    }
}