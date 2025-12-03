// ProfileViewDialog.h
#ifndef PROFILEVIEWDIALOG_H
#define PROFILEVIEWDIALOG_H

#include <QDialog>
#include "UserProfile.h"

class QLabel;
class QPushButton;

class ProfileViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileViewDialog(const UserProfile& profile, QWidget* parent = nullptr);
    void updateProfile(const UserProfile& newProfile);

signals:
    void editRequested();  // 请求编辑的信号

private:
    UserProfile m_profile;

    QLabel* avatarLabel;
    QLabel* nicknameLabel;
    QLabel* emailLabel;
    QLabel* phoneLabel;
    QLabel* passwordLabel;  // 显示为星号
    QPushButton* editButton;

    void setupUI();
    void updateDisplay();
};

#endif // PROFILEVIEWDIALOG_H