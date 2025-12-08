#ifndef PROFILEEDITDIALOG_H
#define PROFILEEDITDIALOG_H

#include <QDialog>
#include "UserProfile.h"
#include <QTextEdit>
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
    void onSignatureModifyClicked();  // 新增
    void onPasswordModifyClicked();  // 新增
    void onPasswordChanged();  // 新增
    void onSignatureChanged();  // 新增

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
    QTextEdit* signatureEdit;  // 使用QTextEdit
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
    QLineEdit* confirmPasswordEdit;  // 新增
    QLabel* confirmPasswordLabel;  // 新增
    QPushButton* signatureModifyBtn;  // 新增
};

#endif // PROFILEEDITDIALOG_H