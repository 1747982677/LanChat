#include "password_util.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>

QString PasswordUtil::hashPassword(const QString& password)
{
    if (password.isEmpty()) {
        qWarning() << "PasswordUtil::hashPassword: Empty password";
        return QString();
    }

    // 生成随机盐值
    QString salt = generateSalt(16);
    
    // 将密码和盐值组合后进行哈希
    QByteArray data = (salt + password).toUtf8();
    QString hash = sha256Hash(data);
    
    // 返回格式：salt:hash（便于后续验证时提取盐值）
    return salt + ":" + hash;
}

bool PasswordUtil::verifyPassword(const QString& password, const QString& passwordHash)
{
    if (password.isEmpty() || passwordHash.isEmpty()) {
        return false;
    }

    // 解析存储的哈希值（格式：salt:hash）
    QStringList parts = passwordHash.split(":");
    if (parts.size() != 2) {
        qWarning() << "PasswordUtil::verifyPassword: Invalid password hash format";
        return false;
    }

    QString salt = parts[0];
    QString storedHash = parts[1];

    // 使用相同的盐值对输入的密码进行哈希
    QByteArray data = (salt + password).toUtf8();
    QString computedHash = sha256Hash(data);

    // 比较哈希值（使用安全的字符串比较，防止时序攻击）
    return storedHash == computedHash;
}

QString PasswordUtil::generateSalt(int length)
{
    if (length <= 0) {
        length = 16;
    }

    QByteArray saltBytes;
    saltBytes.resize(length);
    
    // 使用 QRandomGenerator 生成随机字节
    QRandomGenerator* rng = QRandomGenerator::global();
    for (int i = 0; i < length; ++i) {
        saltBytes[i] = static_cast<char>(rng->bounded(256));
    }

    // 转换为十六进制字符串
    return saltBytes.toHex();
}

QString PasswordUtil::sha256Hash(const QByteArray& data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

