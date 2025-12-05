// UserEntity.h - 用户实体类
#ifndef USERENTITY_H
#define USERENTITY_H

#include <QString>
#include <QPixmap>
#include <QDateTime>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <qpainter.h>

class UserEntity
{
public:
    // 构造函数
    UserEntity()
        : userId("")
        , nickname("新用户")
        , avatarPath("")
        , email("")
        , phone("")
        , signature("")
        , status(0)
        , passwordHash("")
        , lastOnlineTime(0)
    {
        // 初始化默认值
    }

    // 带参数构造函数
    UserEntity(const QString& uid, const QString& nick, const QString& mail,
        const QString& pwdHash, const QString& phoneNum = "", int userStatus = 0)
        : userId(uid)
        , nickname(nick)
        , avatarPath("")
        , email(mail)
        , phone(phoneNum)
        , signature("")
        , status(userStatus)
        , passwordHash(pwdHash)
        , lastOnlineTime(0)
    {
        // 简单验证
        if (userId.isEmpty()) {
            userId = generateUserId();
        }
    }

    // 检查用户信息是否有效
    bool isValid() const
    {
        return !userId.trimmed().isEmpty() &&
            !nickname.trimmed().isEmpty() &&
            !email.trimmed().isEmpty() &&
            !passwordHash.trimmed().isEmpty();
    }

    // 检查是否为有效用户（已注册）
    bool isRegistered() const
    {
        return isValid() && status >= 0;
    }

    // 检查用户是否在线
    bool isOnline() const
    {
        return status == 1; // 假设状态1表示在线
    }

    // 设置在线状态
    void setOnline(bool online)
    {
        status = online ? 1 : 0;
        if (online) {
            updateLastOnlineTime();
        }
    }

    // 更新最后在线时间
    void updateLastOnlineTime()
    {
        lastOnlineTime = QDateTime::currentSecsSinceEpoch();
    }

    // 获取格式化的最后在线时间
    QString getFormattedLastOnlineTime() const
    {
        if (lastOnlineTime == 0) {
            return "从未在线";
        }

        QDateTime lastOnline = QDateTime::fromSecsSinceEpoch(lastOnlineTime);
        QDateTime now = QDateTime::currentDateTime();

        qint64 daysDiff = lastOnline.daysTo(now);

        if (daysDiff == 0) {
            return "今天 " + lastOnline.toString("HH:mm");
        }
        else if (daysDiff == 1) {
            return "昨天 " + lastOnline.toString("HH:mm");
        }
        else if (daysDiff < 7) {
            return QString("%1天前").arg(daysDiff);
        }
        else {
            return lastOnline.toString("yyyy-MM-dd HH:mm");
        }
    }

    // 加载头像（从文件路径）
    QPixmap loadAvatar() const
    {
        if (avatarPath.isEmpty()) {
            return createDefaultAvatar();
        }
        QPixmap pixmap(avatarPath);
        if (pixmap.isNull()) {
            return createDefaultAvatar();
        }
        return pixmap;
    }

    // 设置头像路径
    void setAvatar(const QString& path)
    {
        avatarPath = path;
    }

    // 设置头像（从QPixmap保存到文件）
    bool setAvatar(const QPixmap& pixmap, const QString& saveDir = "avatars/")
    {
        if (pixmap.isNull()) return false;

        // 确保目录存在
        QDir dir(saveDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        // 生成文件名
        QString filename = QString("%1/%2_avatar.png")
            .arg(saveDir)
            .arg(userId.isEmpty() ? "default" : userId);

        // 保存图片
        if (pixmap.save(filename, "PNG")) {
            avatarPath = filename;
            return true;
        }
        return false;
    }

    // 转换为SQL插入语句（统一字段顺序：userId, email, passwordHash, nickname, avatarPath, phone, signature, status, lastOnlineTime）
    QString toInsertSQL() const
    {
        return QString("INSERT INTO users (userId, email, passwordHash, nickname, avatarPath, phone, "
            "signature, status, lastOnlineTime) "
            "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', %8, %9)")
            .arg(userId)
            .arg(email)
            .arg(passwordHash)
            .arg(nickname)
            .arg(avatarPath)
            .arg(phone)
            .arg(signature)
            .arg(status)
            .arg(lastOnlineTime);
    }

    // 转换为SQL更新语句（统一字段顺序）
    QString toUpdateSQL() const
    {
        return QString("UPDATE users SET "
            "email = '%1', "
            "passwordHash = '%2', "
            "nickname = '%3', "
            "avatarPath = '%4', "
            "phone = '%5', "
            "signature = '%6', "
            "status = %7, "
            "lastOnlineTime = %8 "
            "WHERE userId = '%9'")
            .arg(email)
            .arg(passwordHash)
            .arg(nickname)
            .arg(avatarPath)
            .arg(phone)
            .arg(signature)
            .arg(status)
            .arg(lastOnlineTime)
            .arg(userId);
    }
    // 根据 userId 查询的 SQL 语句
    static QString selectByUserIdSQL(const QString& userId)
    {
        return QString("SELECT * FROM users WHERE userId = '%1'").arg(userId);
    }
    // 从数据库行创建用户对象
    static UserEntity fromDatabase(const QMap<QString, QVariant>& row)
    {
        UserEntity user;
        user.userId = row.value("userId").toString();
        user.email = row.value("email").toString();
        user.passwordHash = row.value("passwordHash").toString();
        user.nickname = row.value("nickname").toString();
        user.avatarPath = row.value("avatarPath").toString();
        user.phone = row.value("phone").toString();
        user.signature = row.value("signature").toString();
        user.status = row.value("status").toInt();
        user.lastOnlineTime = row.value("lastOnlineTime").toLongLong();
        return user;
    }

    // 比较两个用户是否相同
    bool operator==(const UserEntity& other) const
    {
        return userId == other.userId;
    }

    // 生成用户ID
    static QString generateUserId()
    {
        return QString("user_%1").arg(QDateTime::currentMSecsSinceEpoch());
    }

    // 清空敏感信息（如密码哈希）
    void clearSensitiveData()
    {
        passwordHash.clear();
    }

public:
    // 成员变量（与数据库字段对应）
    QString userId;          // 用户ID（主键）
    QString email;           // 邮箱（唯一，非空）
    QString passwordHash;    // 密码哈希（非空）
    QString nickname;        // 昵称
    QString avatarPath;      // 头像路径
    QString phone;           // 电话
    QString signature;       // 个性签名
    int status;              // 状态（0:离线, 1:在线, 其他自定义状态）
    qint64 lastOnlineTime;   // 最后在线时间（时间戳）

private:
    // 创建默认头像
    QPixmap createDefaultAvatar() const
    {
        QPixmap pixmap(100, 100);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制圆形背景
        QColor bgColor = generateColorFromString(userId.isEmpty() ? nickname : userId);
        painter.setBrush(bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, 100, 100);

        // 绘制昵称首字母
        if (!nickname.isEmpty()) {
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPointSize(40);
            font.setBold(true);
            painter.setFont(font);

            QString firstChar = nickname.left(1).toUpper();
            painter.drawText(pixmap.rect(), Qt::AlignCenter, firstChar);
        }

        return pixmap;
    }

    // 根据字符串生成颜色
    QColor generateColorFromString(const QString& str) const
    {
        if (str.isEmpty()) return QColor(100, 150, 200);

        // 使用字符串的哈希值生成颜色
        uint hash = qHash(str);
        int r = (hash & 0xFF0000) >> 16;
        int g = (hash & 0x00FF00) >> 8;
        int b = hash & 0x0000FF;

        // 调整亮度，避免太暗或太亮
        r = qBound(50, r, 200);
        g = qBound(50, g, 200);
        b = qBound(50, b, 200);

        return QColor(r, g, b);
    }
};

#endif // USERENTITY_H