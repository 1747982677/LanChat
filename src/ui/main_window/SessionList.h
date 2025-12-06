#ifndef SESSIONLIST_H
#define SESSIONLIST_H

#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QResizeEvent>
#include "SessionInfo.h"


class SessionList : public QListWidget
{
    Q_OBJECT
public:
    explicit SessionList(QWidget* parent = nullptr);
    void loadSessions(const QList<SessionInfo>& list);
    void addSession(const SessionInfo& info);
    void sortSessions();


protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void sessionSelected(const SessionInfo& info);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onContextMenuRequested(const QPoint& pos);

private:
    void handlePinSession(QListWidgetItem* item);
    void handleDeleteSession(QListWidgetItem* item);
};

#endif // SESSIONLIST_H
