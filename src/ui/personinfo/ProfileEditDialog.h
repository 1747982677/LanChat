#ifndef PROFILEEDITDIALOG_H
#define PROFILEEDITDIALOG_H

#include <QDialog>
#include "UserProfile.h"

class QLabel;
class QLineEdit;
class QPushButton;

class ProfileEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileEditDialog(const UserProfile& profile, QWidget* parent = nullptr);

    UserProfile getUpdatedProfile() const;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onAvatarClicked();
    void onSaveClicked();
    void onModifyClicked(QLineEdit* editField, QPushButton* modifyBtn);
    void onEditFieldChanged(QLineEdit* editField, QPushButton* modifyBtn);

private:
    void setupUI();
    QPixmap getRoundedPixmap(const QPixmap& source, int diameter);
    void updateAvatar(const QString& imagePath);

private:
    UserProfile m_originalProfile;
    UserProfile m_updatedProfile;

    // UI元素
    QLabel* avatarLabel;
    QLineEdit* nicknameEdit;
    QLineEdit* emailEdit;
    QLineEdit* phoneEdit;
    QLineEdit* passwordEdit;
    QPushButton* uploadButton;
    QPushButton* nicknameModifyBtn;
    QPushButton* emailModifyBtn;
    QPushButton* phoneModifyBtn;
    QPushButton* passwordModifyBtn;
    QPushButton* saveButton;
    QPushButton* cancelButton;
};

#endif // PROFILEEDITDIALOG_H