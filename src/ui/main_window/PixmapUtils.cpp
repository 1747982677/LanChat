#include "PixmapUtils.h"

QPixmap PixmapUtils::clipToCircle(const QString& srcPath, int radius)
{
    QPixmap src(srcPath);
    // 处理默认头像逻辑
    if (src.isNull()) {
        src.load(":/default/avatar/test1.jpg");
    }
    return clipToCircle(src, radius);
}

QPixmap PixmapUtils::clipToCircle(const QPixmap& src, int radius)
{
    if (src.isNull()) return QPixmap();


    int diameter = radius * 2;

    QPixmap result(diameter, diameter);
    result.fill(Qt::transparent);
    QPainter painter(&result);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, diameter, diameter,
        src.scaled(diameter, diameter,
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation));

    return result;
}
