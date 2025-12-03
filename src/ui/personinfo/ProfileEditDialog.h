// ProfileEditDialog.h
#ifndef PROFILEEDITDIALOG_H
#define PROFILEEDITDIALOG_H

#include <QDialog>
#include "UserProfile.h"

class QLineEdit;
class QPushButton;
class QLabel;

class ProfileEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileEditDialog(const UserProfile& profile, QWidget* parent = nullptr);
    UserProfile getUpdatedProfile() const { return m_updatedProfile; }

private slots:
    void onAvatarClicked();
    void onSaveClicked();
    void onModifyClicked(const QString& field);

private:
    UserProfile m_originalProfile;
    UserProfile m_updatedProfile;

    QLabel* avatarLabel;
    QPushButton* uploadButton;

    QLineEdit* nicknameEdit;
    QPushButton* nicknameModifyBtn;

    QLineEdit* emailEdit;
    QPushButton* emailModifyBtn;

    QLineEdit* phoneEdit;
    QPushButton* phoneModifyBtn;

    QLineEdit* passwordEdit;
    QPushButton* passwordModifyBtn;

    QPushButton* saveButton;

    void setupUI();
    void updateAvatar(const QString& imagePath);
    //bool eventFilter(QObject* obj, QEvent* event) override;  // 关键：声明 eventFilter
};

#endif // PROFILEEDITDIALOG_H