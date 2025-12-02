// NewFriendInfo.cpp
/*
 * 页面名称：新朋友申请同意后弹窗
 * 功能：可修改备注等
 * 用法：通过NewFriendList唤起
 */
#include "NewFriendInfo.h"
#include <QVBoxLayout>
#include <QPixmap>
#include <QHBoxLayout>
#include <QInputDialog>

NewFriendInfo::NewFriendInfo(const QString& name,
    const QString& avatarPath,
    QWidget* parent)
    : QDialog(parent), m_name(name)
{
    this->setFixedSize(300, 380);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    // 头像
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(100, 100);
    m_avatarLabel->setScaledContents(true);
    QPixmap pix(avatarPath);
    if (!pix.isNull()) m_avatarLabel->setPixmap(pix);

    // 昵称
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setStyleSheet("font-size:18px; font-weight:600;");

    // 账号
    m_accountLabel = new QLabel(QStringLiteral("账号：") + name.toLower(), this);
    m_accountLabel->setStyleSheet("color:gray;");

    // 备注（文本框 + 修改按钮）
    auto* remarkLayout = new QHBoxLayout;
    m_contactRemark = new QLineEdit(this);
    m_contactRemark->setEnabled(false);
    m_contactRemark->setText(QStringLiteral("请输入备注"));
    m_contactRemark->setPlaceholderText(QStringLiteral("请输入备注"));
    m_contactRemark->setFixedHeight(30);

    QPushButton* editRemarkBtn = new QPushButton("修改", this);
    editRemarkBtn->setFixedHeight(30);
    editRemarkBtn->setStyleSheet(R"(
        QPushButton {
            background:#3399ff;
            color:white;
            border-radius:4px;
        }
        QPushButton:hover {
            background:#267acc;
        }
    )");

    remarkLayout->addWidget(m_contactRemark);
    remarkLayout->addWidget(editRemarkBtn);

    // 发消息按钮
    m_msgBtn = new QPushButton(QStringLiteral("发消息"), this);
    m_msgBtn->setFixedHeight(40);
    m_msgBtn->setStyleSheet(R"(
        QPushButton {
            background:#07c160;
            color:white;
            border-radius:6px;
            font-size:15px;
        }
        QPushButton:hover {
            background:#06ad56;
        }
    )");

    // 加入布局（顺序很重要：先初始化，再添加）
    layout->addSpacing(20);
    layout->addWidget(m_avatarLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_accountLabel);
    layout->addLayout(remarkLayout);
    layout->addStretch();
    layout->addWidget(m_msgBtn);

    // 信号连接
    connect(editRemarkBtn, &QPushButton::clicked, this, [=]() {
        bool ok;
        QString text = QInputDialog::getText(this, "修改备注", "请输入备注：",
            QLineEdit::Normal,
            m_contactRemark->text(), &ok);
        if (ok && !text.isEmpty()) {
            m_contactRemark->setText(text);
            // TODO: 发信号通知列表更新
        }
        });

    connect(m_msgBtn, &QPushButton::clicked, this, [=]() {
        this->accept();
        });
}



