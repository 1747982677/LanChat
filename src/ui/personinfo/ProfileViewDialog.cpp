// ProfileViewDialog.cpp
#include "ProfileViewDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFont>

ProfileViewDialog::ProfileViewDialog(const UserProfile& profile, QWidget* parent)
    : QDialog(parent), m_profile(profile)
{
    setupUI();
    setWindowTitle("个人信息");
    setFixedSize(350, 400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ProfileViewDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 头像区域
    QHBoxLayout* avatarLayout = new QHBoxLayout();
    avatarLabel = new QLabel(this);
    avatarLabel->setPixmap(m_profile.avatar.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setStyleSheet("border-radius: 40px; border: 2px solid #ccc;");

    QLabel* avatarText = new QLabel("头像", this);
    avatarText->setFont(QFont("微软雅黑", 10));

    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addWidget(avatarText);
    avatarLayout->addStretch();

    // 信息显示区域
    QGridLayout* infoLayout = new QGridLayout();
    infoLayout->setSpacing(15);
    infoLayout->setContentsMargins(20, 0, 20, 0);

    // 昵称
    QLabel* nicknameTitle = new QLabel("昵称", this);
    nicknameTitle->setFont(QFont("微软雅黑", 9));
    nicknameLabel = new QLabel(m_profile.nickname.isEmpty() ? "未设置" : m_profile.nickname, this);
    nicknameLabel->setFont(QFont("微软雅黑", 9));
    infoLayout->addWidget(nicknameTitle, 0, 0);
    infoLayout->addWidget(nicknameLabel, 0, 1, 1, 2);

    // 邮箱
    QLabel* emailTitle = new QLabel("邮箱", this);
    emailTitle->setFont(QFont("微软雅黑", 9));
    emailLabel = new QLabel(m_profile.email.isEmpty() ? "未设置" : m_profile.email, this);
    emailLabel->setFont(QFont("微软雅黑", 9));
    infoLayout->addWidget(emailTitle, 1, 0);
    infoLayout->addWidget(emailLabel, 1, 1, 1, 2);

    // 电话
    QLabel* phoneTitle = new QLabel("电话", this);
    phoneTitle->setFont(QFont("微软雅黑", 9));
    phoneLabel = new QLabel(m_profile.phone.isEmpty() ? "未设置" : m_profile.phone, this);
    phoneLabel->setFont(QFont("微软雅黑", 9));
    infoLayout->addWidget(phoneTitle, 2, 0);
    infoLayout->addWidget(phoneLabel, 2, 1, 1, 2);

    // 密码（显示为星号）
    QLabel* passwordTitle = new QLabel("密码", this);
    passwordTitle->setFont(QFont("微软雅黑", 9));
    passwordLabel = new QLabel("******", this);
    passwordLabel->setFont(QFont("微软雅黑", 9));
    infoLayout->addWidget(passwordTitle, 3, 0);
    infoLayout->addWidget(passwordLabel, 3, 1, 1, 2);

    // 编辑按钮
    editButton = new QPushButton("编辑", this);
    editButton->setFixedSize(80, 30);
    editButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );

    // 添加到主布局
    mainLayout->addLayout(avatarLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(editButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(20);

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
    }

    // 更新文本信息
    nicknameLabel->setText(m_profile.nickname.isEmpty() ? "未设置" : m_profile.nickname);
    emailLabel->setText(m_profile.email.isEmpty() ? "未设置" : m_profile.email);
    phoneLabel->setText(m_profile.phone.isEmpty() ? "未设置" : m_profile.phone);
}