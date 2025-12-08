#ifndef PIXMAPUTILS_H
#define PIXMAPUTILS_H

#include <QPixmap>
#include <QString>
#include <QPainter>
#include <QPainterPath>

class PixmapUtils
{
public:
    /**
     * @brief 生成圆形头像
     * @param srcPath 图片路径
     * @param radius  圆形的半径 (最终图片宽高为 radius * 2)
     * @return 处理好的圆形 QPixmap
     */
    static QPixmap clipToCircle(const QString& srcPath, int radius);

    /**
     * @brief 生成圆形头像 (重载版本，传入 QPixmap)
     * @param src     源图片
     * @param radius  圆形的半径
     * @return 处理好的圆形 QPixmap
     */
    static QPixmap clipToCircle(const QPixmap& src, int radius);
};

#endif // PIXMAPUTILS_H
