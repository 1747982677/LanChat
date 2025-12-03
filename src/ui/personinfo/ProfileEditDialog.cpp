// ProfileEditDialog.cpp
#include "ProfileEditDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QFileInfo>

ProfileEditDialog::ProfileEditDialog(const UserProfile& profile, QWidget* parent)
    : QDialog(parent), m_originalProfile(profile), m_updatedProfile(profile)
{
    setupUI();
    setWindowTitle("编辑个人信息");
    setFixedSize(450, 500);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ProfileEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // 头像区域
    QHBoxLayout* avatarLayout = new QHBoxLayout();
    avatarLabel = new QLabel(this);
    avatarLabel->setPixmap(m_originalProfile.avatar.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    avatarLabel->setFixedSize(100, 100);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   border-radius: 50px;"
        "   border: 2px solid #4CAF50;"
        "}"
    );
    avatarLabel->setCursor(Qt::PointingHandCursor);

    uploadButton = new QPushButton("上传", this);
    uploadButton->setFixedSize(60, 30);
    uploadButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0b7dda;"
        "}"
    );

    QVBoxLayout* avatarRightLayout = new QVBoxLayout();
    QLabel* avatarText = new QLabel("头像", this);
    avatarText->setFont(QFont("微软雅黑", 11, QFont::Bold));
    avatarRightLayout->addWidget(avatarText);
    avatarRightLayout->addWidget(uploadButton);
    avatarRightLayout->addStretch();

    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addLayout(avatarRightLayout);
    avatarLayout->addStretch();

    // 信息编辑区域
    QGridLayout* editLayout = new QGridLayout();
    editLayout->setSpacing(15);
    editLayout->setColumnStretch(1, 1);  // 编辑框列可拉伸

    // 昵称行
    QLabel* nicknameTitle = new QLabel("昵称", this);
    nicknameTitle->setFont(QFont("微软雅黑", 10));
    nicknameEdit = new QLineEdit(m_originalProfile.nickname, this);
    nicknameEdit->setEnabled(false);
    nicknameEdit->setStyleSheet("QLineEdit:disabled { background-color: #f5f5f5; }");

    nicknameModifyBtn = new QPushButton("修改", this);
    nicknameModifyBtn->setFixedSize(60, 30);
    nicknameModifyBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FF9800;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e68900;"
        "}"
    );

    editLayout->addWidget(nicknameTitle, 0, 0);
    editLayout->addWidget(nicknameEdit, 0, 1);
    editLayout->addWidget(nicknameModifyBtn, 0, 2);

    // 邮箱行
    QLabel* emailTitle = new QLabel("邮箱", this);
    emailTitle->setFont(QFont("微软雅黑", 10));
    emailEdit = new QLineEdit(m_originalProfile.email, this);
    emailEdit->setEnabled(false);
    emailEdit->setStyleSheet("QLineEdit:disabled { background-color: #f5f5f5; }");

    emailModifyBtn = new QPushButton("修改", this);
    emailModifyBtn->setFixedSize(60, 30);
    emailModifyBtn->setStyleSheet(nicknameModifyBtn->styleSheet());

    editLayout->addWidget(emailTitle, 1, 0);
    editLayout->addWidget(emailEdit, 1, 1);
    editLayout->addWidget(emailModifyBtn, 1, 2);

    // 电话行
    QLabel* phoneTitle = new QLabel("电话", this);
    phoneTitle->setFont(QFont("微软雅黑", 10));
    phoneEdit = new QLineEdit(m_originalProfile.phone, this);
    phoneEdit->setEnabled(false);
    phoneEdit->setStyleSheet("QLineEdit:disabled { background-color: #f5f5f5; }");

    phoneModifyBtn = new QPushButton("修改", this);
    phoneModifyBtn->setFixedSize(60, 30);
    phoneModifyBtn->setStyleSheet(nicknameModifyBtn->styleSheet());

    editLayout->addWidget(phoneTitle, 2, 0);
    editLayout->addWidget(phoneEdit, 2, 1);
    editLayout->addWidget(phoneModifyBtn, 2, 2);

    // 密码行
    QLabel* passwordTitle = new QLabel("密码", this);
    passwordTitle->setFont(QFont("微软雅黑", 10));
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setEnabled(false);
    passwordEdit->setPlaceholderText("请输入新密码");
    passwordEdit->setStyleSheet("QLineEdit:disabled { background-color: #f5f5f5; }");

    passwordModifyBtn = new QPushButton("修改", this);
    passwordModifyBtn->setFixedSize(60, 30);
    passwordModifyBtn->setStyleSheet(nicknameModifyBtn->styleSheet());

    editLayout->addWidget(passwordTitle, 3, 0);
    editLayout->addWidget(passwordEdit, 3, 1);
    editLayout->addWidget(passwordModifyBtn, 3, 2);

    // 保存按钮
    saveButton = new QPushButton("保存", this);
    saveButton->setFixedSize(100, 40);
    saveButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
    );

    // 添加到主布局
    mainLayout->addLayout(avatarLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(editLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(saveButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(20);

    // 连接信号
    connect(avatarLabel, &QLabel::linkActivated, this, &ProfileEditDialog::onAvatarClicked);
    /*avatarLabel->installEventFilter(this);*/

    connect(uploadButton, &QPushButton::clicked, this, &ProfileEditDialog::onAvatarClicked);

    // 连接修改按钮
    connect(nicknameModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked("nickname"); });
    connect(emailModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked("email"); });
    connect(phoneModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked("phone"); });
    connect(passwordModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked("password"); });

    connect(saveButton, &QPushButton::clicked, this, &ProfileEditDialog::onSaveClicked);
}

void ProfileEditDialog::onAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择头像",
        QDir::homePath(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!fileName.isEmpty()) {
        updateAvatar(fileName);
    }
}

void ProfileEditDialog::onModifyClicked(const QString& field)
{
    if (field == "nickname") {
        nicknameEdit->setEnabled(true);
        nicknameEdit->setFocus();
        nicknameEdit->setStyleSheet("");
    }
    else if (field == "email") {
        emailEdit->setEnabled(true);
        emailEdit->setFocus();
        emailEdit->setStyleSheet("");
    }
    else if (field == "phone") {
        phoneEdit->setEnabled(true);
        phoneEdit->setFocus();
        phoneEdit->setStyleSheet("");
    }
    else if (field == "password") {
        passwordEdit->setEnabled(true);
        passwordEdit->clear();
        passwordEdit->setFocus();
        passwordEdit->setStyleSheet("");
    }
}

void ProfileEditDialog::onSaveClicked()
{
    // 更新个人信息
    m_updatedProfile.nickname = nicknameEdit->text().trimmed();
    m_updatedProfile.email = emailEdit->text().trimmed();
    m_updatedProfile.phone = phoneEdit->text().trimmed();

    QString newPassword = passwordEdit->text().trimmed();
    if (!newPassword.isEmpty()) {
        m_updatedProfile.password = newPassword;
    }

    // 简单验证
    if (m_updatedProfile.nickname.isEmpty()) {
        QMessageBox::warning(this, "警告", "昵称不能为空！");
        nicknameEdit->setFocus();
        return;
    }

    if (m_updatedProfile.email.isEmpty()) {
        QMessageBox::warning(this, "警告", "邮箱不能为空！");
        emailEdit->setFocus();
        return;
    }

    // 邮箱格式验证
    if (!m_updatedProfile.email.contains('@')) {
        QMessageBox::warning(this, "警告", "邮箱格式不正确！");
        emailEdit->setFocus();
        return;
    }

    // 保存成功，关闭对话框
    accept();
}

void ProfileEditDialog::updateAvatar(const QString& imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        m_updatedProfile.avatar = pixmap;
        avatarLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

//// ProfileEditDialog.cpp
//bool ProfileEditDialog::eventFilter(QObject* obj, QEvent* event)
//{
//    if (obj == avatarLabel && event->type() == QEvent::MouseButtonPress) {
//        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
//        if (mouseEvent->button() == Qt::LeftButton) {
//            onAvatarClicked();
//            return true;  // 事件已处理
//        }
//    }
//
//    // 调用正确的父类方法
//    return QObject::eventFilter(obj, event);  // 注意：是 QObject
//    // 或者直接 return false; 让事件继续传递
//}