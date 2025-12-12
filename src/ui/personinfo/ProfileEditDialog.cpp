// ProfileEditDialog.cpp
#include "ProfileEditDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>  // 新增
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QFileInfo>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <qpainterpath.h>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include "UserProfile.h"
#include "utils/password_util.h"
ProfileEditDialog::ProfileEditDialog(const UserProfile& profile, QWidget* parent)
    : QDialog(parent), m_originalProfile(profile), m_updatedProfile(profile)
    , avatarLabel(nullptr)
    , nicknameEdit(nullptr)
    , emailEdit(nullptr)
    , phoneEdit(nullptr)
    , signatureEdit(nullptr)  // 新增
    , passwordEdit(nullptr)
    , confirmPasswordEdit(nullptr)
    , confirmPasswordLabel(nullptr)
    , uploadButton(nullptr)
    , nicknameModifyBtn(nullptr)
    , emailModifyBtn(nullptr)
    , phoneModifyBtn(nullptr)
    , signatureModifyBtn(nullptr)  // 新增
    , passwordModifyBtn(nullptr)
    , saveButton(nullptr)
    , cancelButton(nullptr)
{
    setupUI();
    setWindowTitle("编辑个人信息");
    setFixedSize(500, 550);  // 调整高度
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 安装事件过滤器
    avatarLabel->installEventFilter(this);
}

void ProfileEditDialog::setupUI()
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

    QLabel* titleLabel = new QLabel("编辑个人资料", headerFrame);
    titleLabel->setFont(QFont("微软雅黑", 12, QFont::Bold));
    titleLabel->setStyleSheet("color: white;");

    QPushButton* closeButton = new QPushButton("×", headerFrame);
    closeButton->setFixedSize(24, 24);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: white;"
        "   border: none;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255,255,255,0.2);"
        "   border-radius: 12px;"
        "}"
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &ProfileEditDialog::reject);

    // === 内容区域 ===
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: white;");

    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 25, 30, 25);

    // === 头像区域 ===
    QHBoxLayout* avatarLayout = new QHBoxLayout();

    QVBoxLayout* avatarLeftLayout = new QVBoxLayout();
    avatarLabel = new QLabel(this);

    if (!m_originalProfile.avatar.isNull()) {
        QPixmap roundedAvatar = getRoundedPixmap(m_originalProfile.avatar, 80);
        avatarLabel->setPixmap(roundedAvatar);
    }
    else {
        avatarLabel->setText("点击上传");
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setFont(QFont("微软雅黑", 9));
        avatarLabel->setStyleSheet("color: #888;");
    }

    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #F0F0F0;"
        "   border: 2px dashed #CCCCCC;"
        "   border-radius: 40px;"
        "}"
    );
    avatarLabel->setCursor(Qt::PointingHandCursor);

    uploadButton = new QPushButton("更换头像", this);
    uploadButton->setFixedSize(80, 28);
    uploadButton->setFont(QFont("微软雅黑", 9));
    uploadButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #E8F5E9;"
        "   color: #4CAF50;"
        "   border: 1px solid #C8E6C9;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #C8E6C9;"
        "   border-color: #A5D6A7;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #A5D6A7;"
        "}"
    );

    avatarLeftLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
    avatarLeftLayout->addWidget(uploadButton, 0, Qt::AlignCenter);

    QVBoxLayout* avatarRightLayout = new QVBoxLayout();
    QLabel* avatarTips = new QLabel("支持 JPG、PNG 格式\n建议尺寸 200×200 像素\n文件大小不超过 2MB", this);
    avatarTips->setFont(QFont("微软雅黑", 9));
    avatarTips->setStyleSheet("color: #999; line-height: 1.4;");
    avatarTips->setAlignment(Qt::AlignLeft);
    avatarRightLayout->addStretch();
    avatarRightLayout->addWidget(avatarTips);
    avatarRightLayout->addStretch();

    avatarLayout->addLayout(avatarLeftLayout);
    avatarLayout->addSpacing(20);
    avatarLayout->addLayout(avatarRightLayout);
    avatarLayout->addStretch();

    // === 信息编辑区域 ===
    QFrame* editFrame = new QFrame(this);
    editFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #FAFAFA;"
        "   border: 1px solid #EEE;"
        "   border-radius: 8px;"
        "}"
    );

    QVBoxLayout* editLayout = new QVBoxLayout(editFrame);
    editLayout->setSpacing(1);
    editLayout->setContentsMargins(20, 15, 20, 15);

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
            "   margin: 8px 0;"
            "}"
        );
        return line;
        };

    // 创建编辑行函数
    auto createEditRow = [&](const QString& title, QLineEdit*& editField, QPushButton*& modifyBtn,
        const QString& value, bool isPassword = false) -> QHBoxLayout* {
            QHBoxLayout* rowLayout = new QHBoxLayout();
            rowLayout->setSpacing(10);

            // 标题
            QLabel* titleLabel = new QLabel(title, editFrame);
            titleLabel->setFont(QFont("微软雅黑", 10, QFont::Medium));
            titleLabel->setStyleSheet("color: #555; min-width: 60px;");
            titleLabel->setAlignment(Qt::AlignCenter);//Qt::AlignRight | Qt::AlignVCenter

            // 编辑框
            editField = new QLineEdit(value, editFrame);
            editField->setFont(QFont("微软雅黑", 10));
            editField->setStyleSheet(
                "QLineEdit {"
                "   border: 1px solid #DDD;"
                "   border-radius: 4px;"
                "   padding: 8px 12px;"
                "   background-color: white;"
                "}"
                "QLineEdit:focus {"
                "   border-color: #4CAF50;"
                "   background-color: #FFFFE0;"
                "}"
                "QLineEdit:disabled {"
                "   background-color: #F5F5F5;"
                "   color: #666;"
                "}"
            );

            if (isPassword) {
                editField->setEchoMode(QLineEdit::Password);
                editField->setPlaceholderText("请输入新密码");
            }

            editField->setEnabled(false);

            // 修改按钮
            modifyBtn = new QPushButton("修改", editFrame);
            modifyBtn->setFixedSize(60, 30);
            modifyBtn->setFont(QFont("微软雅黑", 9));
            modifyBtn->setStyleSheet(
                "QPushButton {"
                "   background-color: #FF9800;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 4px;"
                "}"
                "QPushButton:hover {"
                "   background-color: #F57C00;"
                "}"
                "QPushButton:pressed {"
                "   background-color: #E65100;"
                "}"
                "QPushButton:disabled {"
                "   background-color: #FFCC80;"
                "}"
            );

            rowLayout->addWidget(titleLabel);
            rowLayout->addWidget(editField, 1);
            rowLayout->addWidget(modifyBtn);

            return rowLayout;
        };

    // 昵称行
    editLayout->addLayout(createEditRow("昵称", nicknameEdit, nicknameModifyBtn, m_originalProfile.nickname));
    editLayout->addWidget(createGradientLine(editFrame));

    // 邮箱行
    editLayout->addLayout(createEditRow("邮箱", emailEdit, emailModifyBtn, m_originalProfile.email));
    editLayout->addWidget(createGradientLine(editFrame));

    // 电话行
    editLayout->addLayout(createEditRow("电话", phoneEdit, phoneModifyBtn, m_originalProfile.phone));
    editLayout->addWidget(createGradientLine(editFrame));

    // === 签名行 ===
    QHBoxLayout* signatureLayout = new QHBoxLayout();
    signatureLayout->setSpacing(10);

    QLabel* signatureLabel = new QLabel("签名", editFrame);
    signatureLabel->setFont(QFont("微软雅黑", 10, QFont::Medium));
    signatureLabel->setStyleSheet("color: #555; min-width: 60px;");
    signatureLabel->setAlignment(Qt::AlignCenter);

    signatureEdit = new QTextEdit(m_originalProfile.signure, editFrame);  // 使用QTextEdit
    signatureEdit->setFont(QFont("微软雅黑", 10));
    signatureEdit->setMaximumHeight(60);
    signatureEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 1px solid #DDD;"
        "   border-radius: 4px;"
        "   padding: 8px 12px;"
        "   background-color: #F5F5F5;"
        "   color: #666;"
        "   max-height: 60px;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #4CAF50;"
        "   background-color: #FFFFE0;"
        "}"
        "QTextEdit:disabled {"
        "   background-color: #F5F5F5;"
        "   color: #666;"
        "}"
    );
    signatureEdit->setEnabled(false);

    signatureModifyBtn = new QPushButton("修改", editFrame);
    signatureModifyBtn->setFixedSize(60, 30);
    signatureModifyBtn->setFont(QFont("微软雅黑", 9));
    signatureModifyBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #FF9800;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F57C00;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #E65100;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #FFCC80;"
        "}"
    );

    signatureLayout->addWidget(signatureLabel);
    signatureLayout->addWidget(signatureEdit, 1);
    signatureLayout->addWidget(signatureModifyBtn);

    editLayout->addLayout(signatureLayout);
    editLayout->addWidget(createGradientLine(editFrame));

    // 密码行
    editLayout->addLayout(createEditRow("密码", passwordEdit, passwordModifyBtn, "", true));

    // 确认密码行（初始隐藏）
    QHBoxLayout* confirmPasswordLayout = new QHBoxLayout();
    confirmPasswordLayout->setSpacing(10);

    confirmPasswordLabel = new QLabel("确认密码", editFrame);
    confirmPasswordLabel->setFont(QFont("微软雅黑", 10, QFont::Medium));
    confirmPasswordLabel->setStyleSheet("color: #555; min-width: 60px;");
    confirmPasswordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    confirmPasswordLabel->setVisible(false);

    confirmPasswordEdit = new QLineEdit(editFrame);
    confirmPasswordEdit->setFont(QFont("微软雅黑", 10));
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    confirmPasswordEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #DDD;"
        "   border-radius: 4px;"
        "   padding: 8px 12px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #4CAF50;"
        "   background-color: #FFFFE0;"
        "}"
    );
    confirmPasswordEdit->setVisible(false);

    QLabel* placeholderLabel = new QLabel(editFrame);
    placeholderLabel->setFixedSize(60, 30);
    placeholderLabel->setVisible(false);

    confirmPasswordLayout->addWidget(confirmPasswordLabel);
    confirmPasswordLayout->addWidget(confirmPasswordEdit, 1);
    confirmPasswordLayout->addWidget(placeholderLabel);

    editLayout->addLayout(confirmPasswordLayout);

    // === 按钮区域 ===
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

    cancelButton = new QPushButton("取消", this);
    cancelButton->setFixedSize(120, 36);
    cancelButton->setFont(QFont("微软雅黑", 10));
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #F5F5F5;"
        "   color: #666;"
        "   border: 1px solid #DDD;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E0E0E0;"
        "   border-color: #CCC;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #D6D6D6;"
        "}"
    );

    saveButton = new QPushButton("保存", this);
    saveButton->setFixedSize(120, 36);
    saveButton->setFont(QFont("微软雅黑", 10, QFont::Bold));
    saveButton->setStyleSheet(
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

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();

    // === 组装内容区域 ===
    contentLayout->addLayout(avatarLayout);
    contentLayout->addWidget(editFrame);
    contentLayout->addLayout(buttonLayout);

    // === 组装主布局 ===
    mainLayout->addWidget(headerFrame);
    mainLayout->addWidget(contentWidget);

    // === 连接信号 ===
    connect(uploadButton, &QPushButton::clicked, this, &ProfileEditDialog::onAvatarClicked);

    // 修改按钮连接
    connect(nicknameModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked(nicknameEdit, nicknameModifyBtn); });
    connect(emailModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked(emailEdit, emailModifyBtn); });
    connect(phoneModifyBtn, &QPushButton::clicked, this, [this]() { onModifyClicked(phoneEdit, phoneModifyBtn); });
    connect(signatureModifyBtn, &QPushButton::clicked, this, &ProfileEditDialog::onSignatureModifyClicked);
    connect(passwordModifyBtn, &QPushButton::clicked, this, &ProfileEditDialog::onPasswordModifyClicked);

    // 编辑框内容改变时更新按钮状态
    connect(nicknameEdit, &QLineEdit::textChanged, this, [this]() { onEditFieldChanged(nicknameEdit, nicknameModifyBtn); });
    connect(emailEdit, &QLineEdit::textChanged, this, [this]() { onEditFieldChanged(emailEdit, emailModifyBtn); });
    connect(phoneEdit, &QLineEdit::textChanged, this, [this]() { onEditFieldChanged(phoneEdit, phoneModifyBtn); });
    connect(signatureEdit, &QTextEdit::textChanged, this, &ProfileEditDialog::onSignatureChanged);
    connect(passwordEdit, &QLineEdit::textChanged, this, &ProfileEditDialog::onPasswordChanged);
    connect(confirmPasswordEdit, &QLineEdit::textChanged, this, &ProfileEditDialog::onPasswordChanged);

    connect(cancelButton, &QPushButton::clicked, this, &ProfileEditDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &ProfileEditDialog::onSaveClicked);
}

QPixmap ProfileEditDialog::getRoundedPixmap(const QPixmap& source, int diameter)
{
    if (source.isNull()) {
        return QPixmap();
    }

    QPixmap scaledPixmap = source.scaled(diameter, diameter, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPixmap roundedPixmap(diameter, diameter);
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaledPixmap);

    return roundedPixmap;
}

void ProfileEditDialog::onAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择头像",
        QDir::homePath(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (fileInfo.size() > 2 * 1024 * 1024) { // 2MB限制
            QMessageBox::warning(this, "警告", "图片大小不能超过 2MB！");
            return;
        }

        updateAvatar(fileName);
    }
}

void ProfileEditDialog::onModifyClicked(QLineEdit* editField, QPushButton* modifyBtn)
{
    if (!editField->isEnabled()) {
        editField->setEnabled(true);
        editField->setFocus();
        editField->selectAll();
        modifyBtn->setText("取消");
        modifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #F44336;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #D32F2F;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #B71C1C;"
            "}"
        );
    }
    else {
        // 恢复原值
        QString originalValue;
        if (editField == nicknameEdit) {
            originalValue = m_originalProfile.nickname;
        }
        else if (editField == emailEdit) {
            originalValue = m_originalProfile.email;
        }
        else if (editField == phoneEdit) {
            originalValue = m_originalProfile.phone;
        }

        editField->setText(originalValue);
        editField->setEnabled(false);
        editField->setStyleSheet(
            "QLineEdit {"
            "   border: 1px solid #DDD;"
            "   border-radius: 4px;"
            "   padding: 8px 12px;"
            "   background-color: #F5F5F5;"
            "   color: #666;"
            "}"
        );

        modifyBtn->setText("修改");
        modifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF9800;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #F57C00;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #E65100;"
            "}"
        );
    }
}

void ProfileEditDialog::onSignatureModifyClicked()
{
    if (!signatureEdit->isEnabled()) {
        signatureEdit->setEnabled(true);
        signatureEdit->setFocus();
        signatureModifyBtn->setText("取消");
        signatureModifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #F44336;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #D32F2F;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #B71C1C;"
            "}"
        );
    }
    else {
        signatureEdit->setText(m_originalProfile.signure);
        signatureEdit->setEnabled(false);
        signatureEdit->setStyleSheet(
            "QTextEdit {"
            "   border: 1px solid #DDD;"
            "   border-radius: 4px;"
            "   padding: 8px 12px;"
            "   background-color: #F5F5F5;"
            "   color: #666;"
            "   max-height: 60px;"
            "}"
        );

        signatureModifyBtn->setText("修改");
        signatureModifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF9800;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #F57C00;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #E65100;"
            "}"
        );
    }
}

void ProfileEditDialog::onPasswordModifyClicked()
{
    if (!passwordEdit->isEnabled()) {
        // 启用密码编辑并显示确认密码行
        passwordEdit->setEnabled(true);
        passwordEdit->setFocus();
        passwordEdit->selectAll();

        // 显示确认密码行
        confirmPasswordLabel->setVisible(true);
        confirmPasswordEdit->setVisible(true);
        confirmPasswordEdit->setEnabled(true);
        confirmPasswordEdit->clear();

        passwordModifyBtn->setText("取消");
        passwordModifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #F44336;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #D32F2F;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #B71C1C;"
            "}"
        );

        // 调整对话框高度
        this->setFixedSize(500, 590);  // 增加高度
    }
    else {
        // 取消密码修改
        passwordEdit->setText("");
        passwordEdit->setEnabled(false);

        // 隐藏确认密码行
        confirmPasswordLabel->setVisible(false);
        confirmPasswordEdit->setVisible(false);
        confirmPasswordEdit->clear();

        passwordEdit->setStyleSheet(
            "QLineEdit {"
            "   border: 1px solid #DDD;"
            "   border-radius: 4px;"
            "   padding: 8px 12px;"
            "   background-color: #F5F5F5;"
            "   color: #666;"
            "}"
        );

        passwordModifyBtn->setText("修改");
        passwordModifyBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #FF9800;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #F57C00;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #E65100;"
            "}"
        );

        // 恢复对话框高度
        this->setFixedSize(500, 550);
    }
}

void ProfileEditDialog::onEditFieldChanged(QLineEdit* editField, QPushButton* modifyBtn)
{
    if (editField->isEnabled() && modifyBtn->text() == "取消") {
        // 如果编辑框有内容，启用保存按钮
        bool hasChanges = !editField->text().trimmed().isEmpty();
        if (hasChanges) {
            saveButton->setEnabled(true);
        }
    }
}

void ProfileEditDialog::onSignatureChanged()
{
    if (signatureEdit->isEnabled() && signatureModifyBtn->text() == "取消") {
        saveButton->setEnabled(true);
    }
}

void ProfileEditDialog::onPasswordChanged()
{
    // 检查密码一致性
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 只有确认密码行可见时才验证一致性
    if (confirmPasswordEdit->isVisible()) {
        if (!password.isEmpty() && !confirmPassword.isEmpty()) {
            if (password != confirmPassword) {
                // 密码不一致，显示错误样式
                passwordEdit->setStyleSheet(
                    "QLineEdit {"
                    "   border: 1px solid #FF5722;"
                    "   border-radius: 4px;"
                    "   padding: 8px 12px;"
                    "   background-color: #FFF8F7;"
                    "}"
                );

                confirmPasswordEdit->setStyleSheet(
                    "QLineEdit {"
                    "   border: 1px solid #FF5722;"
                    "   border-radius: 4px;"
                    "   padding: 8px 12px;"
                    "   background-color: #FFF8F7;"
                    "}"
                );

                saveButton->setEnabled(false);
                return;
            }
            else {
                // 密码一致，恢复正常样式
                passwordEdit->setStyleSheet(
                    "QLineEdit {"
                    "   border: 1px solid #4CAF50;"
                    "   border-radius: 4px;"
                    "   padding: 8px 12px;"
                    "   background-color: #F8FFF8;"
                    "}"
                );

                confirmPasswordEdit->setStyleSheet(
                    "QLineEdit {"
                    "   border: 1px solid #4CAF50;"
                    "   border-radius: 4px;"
                    "   padding: 8px 12px;"
                    "   background-color: #F8FFF8;"
                    "}"
                );

                saveButton->setEnabled(true);
                return;
            }
        }
        else if (!password.isEmpty() && confirmPassword.isEmpty()) {
            // 只输入了密码，未输入确认密码
            passwordEdit->setStyleSheet(
                "QLineEdit {"
                "   border: 1px solid #FF9800;"
                "   border-radius: 4px;"
                "   padding: 8px 12px;"
                "   background-color: #FFFBF0;"
                "}"
            );

            confirmPasswordEdit->setStyleSheet(
                "QLineEdit {"
                "   border: 1px solid #DDD;"
                "   border-radius: 4px;"
                "   padding: 8px 12px;"
                "   background-color: white;"
                "}"
            );

            saveButton->setEnabled(false);
            return;
        }
    }

    // 默认样式
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #DDD;"
        "   border-radius: 4px;"
        "   padding: 8px 12px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #4CAF50;"
        "   background-color: #FFFFE0;"
        "}"
    );

    if (confirmPasswordEdit->isVisible()) {
        confirmPasswordEdit->setStyleSheet(
            "QLineEdit {"
            "   border: 1px solid #DDD;"
            "   border-radius: 4px;"
            "   padding: 8px 12px;"
            "   background-color: white;"
            "}"
            "QLineEdit:focus {"
            "   border-color: #4CAF50;"
            "   background-color: #FFFFE0;"
            "}"
        );
    }

    // 检查其他字段是否有变化
    bool hasOtherChanges = (
        nicknameEdit->text() != m_originalProfile.nickname ||
        emailEdit->text() != m_originalProfile.email ||
        phoneEdit->text() != m_originalProfile.phone ||
        signatureEdit->toPlainText() != m_originalProfile.signure ||
        !password.isEmpty()
        );

    saveButton->setEnabled(hasOtherChanges);
}

void ProfileEditDialog::onSaveClicked()
{
    // 收集更新后的信息
    m_updatedProfile.nickname = nicknameEdit->text().trimmed();
    m_updatedProfile.email = emailEdit->text().trimmed();
    m_updatedProfile.phone = phoneEdit->text().trimmed();
    m_updatedProfile.signure = signatureEdit->toPlainText().trimmed();  // 新增

    QString newPassword = passwordEdit->text().trimmed();
    QString confirmPassword = confirmPasswordEdit->text().trimmed();

    // 如果密码被修改，需要验证
    if (!newPassword.isEmpty()) {
        // 验证密码长度
        if (newPassword.length() < 6) {
            QMessageBox::warning(this, "警告", "密码长度不能少于6位！");
            passwordEdit->setFocus();
            passwordEdit->selectAll();
            return;
        }

        // 如果确认密码行可见，验证一致性
        if (confirmPasswordEdit->isVisible()) {
            if (newPassword != confirmPassword) {
                QMessageBox::warning(this, "警告", "两次输入的密码不一致！");
                confirmPasswordEdit->setFocus();
                confirmPasswordEdit->selectAll();
                return;
            }
        }
        m_updatedProfile.password = PasswordUtil::hashPassword(newPassword);
    }

    // 验证必填项
    if (m_updatedProfile.nickname.isEmpty()) {
        QMessageBox::warning(this, "警告", "昵称不能为空！");
        if (!nicknameEdit->isEnabled()) {
            onModifyClicked(nicknameEdit, nicknameModifyBtn);
        }
        nicknameEdit->setFocus();
        return;
    }

    if (m_updatedProfile.email.isEmpty()) {
        QMessageBox::warning(this, "警告", "邮箱不能为空！");
        if (!emailEdit->isEnabled()) {
            onModifyClicked(emailEdit, emailModifyBtn);
        }
        emailEdit->setFocus();
        return;
    }

    // 邮箱格式验证
    QRegularExpression emailRegex(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)");
    if (!emailRegex.match(m_updatedProfile.email).hasMatch()) {
        QMessageBox::warning(this, "警告", "邮箱格式不正确！");
        if (!emailEdit->isEnabled()) {
            onModifyClicked(emailEdit, emailModifyBtn);
        }
        emailEdit->setFocus();
        emailEdit->selectAll();
        return;
    }

    // 手机号格式验证（可选）
    if (!m_updatedProfile.phone.isEmpty()) {
        QRegularExpression phoneRegex(R"(^1[3-9]\d{9}$)");
        if (!phoneRegex.match(m_updatedProfile.phone).hasMatch()) {
            QMessageBox::warning(this, "警告", "手机号格式不正确！");
            if (!phoneEdit->isEnabled()) {
                onModifyClicked(phoneEdit, phoneModifyBtn);
            }
            phoneEdit->setFocus();
            phoneEdit->selectAll();
            return;
        }
    }

    // 保存成功
    accept();
}

void ProfileEditDialog::updateAvatar(const QString& imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        QPixmap roundedPixmap = getRoundedPixmap(pixmap, 80);
        m_updatedProfile.avatar = pixmap;
        avatarLabel->setPixmap(roundedPixmap);
        avatarLabel->setText("");

        // 1. 复制到assets目录
        QString assetsPath = m_updatedProfile.rootpath;
        QFileInfo fileInfo(imagePath);
        QString suffix = fileInfo.suffix();
        QString userpicname = "/ui/assets/" + m_updatedProfile.userid + "." + suffix;
        QString targetPath = assetsPath + userpicname;

        // 删除旧文件（如果存在）
        if (QFile::exists(targetPath)) {
            QFile::remove(targetPath);
        }

        // 复制新文件
        if (QFile::copy(imagePath, targetPath)) {
            m_updatedProfile.avatarpath = userpicname;
        }

        avatarLabel->setStyleSheet(
            "QLabel {"
            "   background-color: transparent;"
            "   border: none;"
            "   border-radius: 40px;"
            "}"
        );
    }
    else {
        QMessageBox::warning(this, "错误", "无法加载图片文件！");
    }
}

bool ProfileEditDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == avatarLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onAvatarClicked();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

UserProfile ProfileEditDialog::getUpdatedProfile() const
{
    return m_updatedProfile;
}