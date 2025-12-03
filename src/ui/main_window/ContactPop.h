#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class ContactPop : public QWidget
{
    Q_OBJECT
public:
    explicit ContactPop(const QString& name,
        const QString& avatarPath,
        QWidget* parent = nullptr)
        : QWidget(parent, Qt::ToolTip) // 悬浮窗类型
    {
        this->setAttribute(Qt::WA_ShowWithoutActivating);
        this->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
        this->setFixedSize(200, 120);

        auto* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);

        QLabel* avatar = new QLabel(this);
        avatar->setFixedSize(50, 50);
        avatar->setScaledContents(true);
        QPixmap pix(avatarPath);
        if (!pix.isNull()) avatar->setPixmap(pix);

        QLabel* nameLabel = new QLabel(name, this);
        nameLabel->setStyleSheet("font-weight:bold; font-size:14px;");

        QLabel* accountLabel = new QLabel(QStringLiteral("账号：") + name.toLower(), this);
        accountLabel->setStyleSheet("color:gray; font-size:12px;");

        layout->addWidget(avatar, 0, Qt::AlignHCenter);
        layout->addWidget(nameLabel, 0, Qt::AlignHCenter);
        layout->addWidget(accountLabel, 0, Qt::AlignHCenter);
    }
};
