#ifndef NCHATMESSAGEITEM_H
#define NCHATMESSAGEITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSize>

class NChatMessageItem : public QWidget
{
    Q_OBJECT
public:
    explicit NChatMessageItem(bool isSelf, const QString& text, const QString& avatarPath, QWidget* parent = nullptr);
    void adjustContent(int width);
    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUi();

    bool m_isSelf;
    QString m_text;
    QString m_avatarPath;

    QLabel* m_lblAvatar;
    QLabel* m_lblBubble;
    QHBoxLayout* m_mainLayout;
};

#endif // NCHATMESSAGEITEM_H
