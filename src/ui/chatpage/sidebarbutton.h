#ifndef SIDEBARBUTTON_H
#define SIDEBARBUTTON_H

#include <QToolButton>
#include <QString>

class SideBarButton: public QToolButton
{
    Q_OBJECT
public:
    explicit SideBarButton(const QString &text, const QString &normalIconPath,
                           const QString &activeIconPath,
                           QWidget *parent = nullptr);
};

#endif // SIDEBARBUTTON_H
