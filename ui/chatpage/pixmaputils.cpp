#include "pixmaputils.h"

QPixmap PixmapUtils::clipToCircle(const QString &srcPath, int radius)
{
    QPixmap src(srcPath);
    // 处理默认头像逻辑
    if (src.isNull()) {
        src.load(":/img/member1.jpg");
    }
    return clipToCircle(src, radius);
}

QPixmap PixmapUtils::clipToCircle(const QPixmap &src, int radius)
{
    if (src.isNull()) return QPixmap();

    // 目标尺寸 (直径)
    int diameter = radius * 2;

    // 创建一个透明的画布
    QPixmap result(diameter, diameter);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    // 开启抗锯齿，保证边缘平滑
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // 设置裁剪路径为圆形
    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    painter.setClipPath(path);

    // 将原图缩放并绘制到中心
    painter.drawPixmap(0, 0, diameter, diameter,
                       src.scaled(diameter, diameter,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation));

    return result;
}
