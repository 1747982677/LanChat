#include "SessionList.h"
#include "SessionListItem.h"
#include "DataLoader.h"
#include <QScrollBar>
#include <QFile>
#include <QDebug>
#include <algorithm>
#include <QScroller>
#include <QScrollerProperties>
#include <QVariant>

SessionList::SessionList(QWidget* parent) : QListWidget(parent)
{
    this->setFrameShape(QFrame::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setMinimumWidth(300);

    this->setStyleSheet(
        "QListWidget {"
        "   background-color: transparent;"
        "   border: none;"
        "   outline: 0;"
        "}"
        "QListWidget::item {"
        "   border-radius: 8px;"
        "   margin: 2px 5px;"
        "}"
        "QListWidget::item:selected, QListWidget::item:selected:!active {"
        "   background-color: #e6e6e6;"
        "   color: black;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #f0f0f0;"
        "}"
        "QListWidget::item:selected:hover {"
        "   background-color: #e6e6e6;"
        "}"
        "QListWidget QWidget {"
        "   background-color: transparent;"
        "}"
        "QListWidget QLabel {"
        "   background-color: transparent;"
        "}"

        // 滚动条优化
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: transparent;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #C1C1C1;"
        "   min-height: 30px;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #A8A8A8;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "   subcontrol-position: bottom;"
        "   subcontrol-origin: margin;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "   background: none;"
        "}"
    );

    connect(this, &QListWidget::itemClicked, this, &SessionList::onItemClicked);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, this, &SessionList::onContextMenuRequested);

}

void SessionList::loadSessions(const QList<SessionInfo>& list)
{
    this->setUpdatesEnabled(false);
    this->clear();

    QList<SessionInfo> sortedList = list;

    // 排序：置顶优先 > 时间倒序
    std::sort(sortedList.begin(), sortedList.end(), [](const SessionInfo& a, const SessionInfo& b) {
        if (a.isTop() != b.isTop()) {
            return a.isTop() > b.isTop(); // 置顶排前面
        }
        return a.lastTime() > b.lastTime();
        });

    for (const SessionInfo& info : sortedList) {
        addSession(info);
    }
    this->setUpdatesEnabled(true);
}

void SessionList::addSession(const SessionInfo& info)
{
    QListWidgetItem* item = new QListWidgetItem(this);
    item->setSizeHint(QSize(this->width(), 70));

    SessionListItem* widget = new SessionListItem();
    widget->setData(info);

    this->setItemWidget(item, widget);
}

void SessionList::upsertSession(const SessionInfo& info)
{
    // 查找是否已有同 uid 的会话，若有则更新数据后返回
    for (int i = 0; i < this->count(); ++i) {
        QWidget* w = this->itemWidget(this->item(i));
        SessionListItem* sItem = qobject_cast<SessionListItem*>(w);
        if (sItem && sItem->getData().uid() == info.uid()) {
            sItem->setData(info);
            return;
        }
    }
    addSession(info);
}

void SessionList::onItemClicked(QListWidgetItem* item)
{
    QWidget* w = this->itemWidget(item);
    SessionListItem* sessionWidget = qobject_cast<SessionListItem*>(w);

    if (sessionWidget) {
        SessionInfo info = sessionWidget->getData();

        info.clearUnread();
        sessionWidget->setData(info);

        emit sessionSelected(info);
    }
}

void SessionList::onContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* item = itemAt(pos);
    if (!item) return;

    QMenu menu(this);

    menu.setAttribute(Qt::WA_TranslucentBackground);
    menu.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);

    menu.setStyleSheet(
        "QMenu {"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    padding: 4px 0px;"
        "}"
        "QMenu::item {"
        "    background-color: transparent;"
        "    padding: 6px 24px;"
        "    font-size: 13px;"
        "    color: #333333;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #F5F5F5;"
        "    color: #000000;"
        "}"
    );

    QWidget* w = this->itemWidget(item);
    SessionListItem* sessionItem = qobject_cast<SessionListItem*>(w);
    bool currentIsTop = false;

    if (sessionItem) {
        currentIsTop = sessionItem->getData().isTop();
    }

    QAction* pinAction = menu.addAction(currentIsTop ? "取消置顶" : "置顶会话");
    QAction* deleteAction = menu.addAction("删除会话");

    QAction* selectedAction = menu.exec(mapToGlobal(pos));

    if (selectedAction == deleteAction) {
        handleDeleteSession(item);
    }
    else if (selectedAction == pinAction) {
        handlePinSession(item);
    }
}

void SessionList::handleDeleteSession(QListWidgetItem* item)
{
    if (!item) return;
    int row = this->row(item);
    delete this->takeItem(row);
    sortSessions();
}

void SessionList::handlePinSession(QListWidgetItem* item)
{
    QWidget* w = this->itemWidget(item);
    SessionListItem* widget = qobject_cast<SessionListItem*>(w);
    if (!widget) return;

    SessionInfo info = widget->getData();
    info.setTop(!info.isTop()); // 切换置顶状态
    widget->setData(info);

    sortSessions();
}

void SessionList::sortSessions()
{
    QList<SessionInfo> allSessions;
    for (int i = 0; i < this->count(); ++i) {
        QWidget* w = this->itemWidget(this->item(i));
        SessionListItem* sItem = qobject_cast<SessionListItem*>(w);
        if (sItem) {
            allSessions.append(sItem->getData());
        }
    }
    this->loadSessions(allSessions);
}

void SessionList::resizeEvent(QResizeEvent* event)
{
    QListWidget::resizeEvent(event);
    int newWidth = this->viewport()->width();

    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        item->setSizeHint(QSize(newWidth, 72));
    }
}
