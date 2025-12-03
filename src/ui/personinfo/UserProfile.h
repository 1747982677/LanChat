// UserProfile.h - 直接在头文件中实现
#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QString>
#include <QPixmap>

class UserProfile
{
public:
    // 构造函数
    UserProfile()
        : nickname("新用户")
        , email("")
        , phone("")
        , password("")
    {
        // 创建默认头像
        avatar = QPixmap(100, 100);
        avatar.fill(QColor(100, 150, 200));
    }

    // 检查信息是否有效
    bool isValid() const
    {
        return !nickname.trimmed().isEmpty() && !email.trimmed().isEmpty();
    }

    // 保存/加载到文件（暂时不实现）
    bool saveToFile(const QString& filename) { return false; }
    bool loadFromFile(const QString& filename) { return false; }

    // 用户信息
    QString nickname;      // 昵称
    QString email;         // 邮箱
    QString phone;         // 电话
    QString password;      // 密码
    QPixmap avatar;        // 头像
};

#endif // USERPROFILE_H