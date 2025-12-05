#ifndef PASSWORD_UTIL_H
#define PASSWORD_UTIL_H

#include <QString>

/**
 * @brief 密码工具类
 * 
 * 提供密码哈希和验证功能，使用 SHA-256 + Salt
 */
class PasswordUtil
{
public:
    /**
     * @brief 生成密码哈希值（SHA-256 + Salt）
     * @param password 明文密码
     * @return 哈希值（十六进制字符串，格式：salt:hash）
     */
    static QString hashPassword(const QString& password);

    /**
     * @brief 验证密码
     * @param password 明文密码
     * @param passwordHash 存储的哈希值（格式：salt:hash）
     * @return 是否匹配
     */
    static bool verifyPassword(const QString& password, const QString& passwordHash);

    /**
     * @brief 生成随机盐值
     * @param length 盐值长度（默认16字节）
     * @return 十六进制字符串
     */
    static QString generateSalt(int length = 16);

private:
    /**
     * @brief SHA-256 哈希
     * @param data 待哈希的数据
     * @return 十六进制字符串
     */
    static QString sha256Hash(const QByteArray& data);
};

#endif // PASSWORD_UTIL_H

