#ifndef NCHATMESSAGEITEM_H
#define NCHATMESSAGEITEM_H
#include "messagedata.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QResizeEvent>

class NChatMessageItem: public QWidget
{
    Q_OBJECT
public:
    explicit NChatMessageItem(MsgRole role, const QString &text, const QString &avatarPath, QWidget *parent = nullptr);
    // 用于获取为了正确显示该Item所需的高度（ListWidget需要）
    static int estimateHeight(const QString &text, int width);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void initUi();

    MsgRole m_role;
    QString m_text;
    QString m_avatarPath;

    QLabel *m_lblAvatar;
    QLabel *m_lblBubble;
};

#endif // NCHATMESSAGEITEM_H
