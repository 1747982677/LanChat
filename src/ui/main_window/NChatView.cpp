#include "NChatview.h"
#include "NChatMessageItem.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QDebug>
#include <QTimer>

NChatView::NChatView(QWidget* parent) : QListWidget(parent)
{
    // --- 样式设置  ---
    this->setFrameShape(QFrame::NoFrame);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyleSheet(R"(
        QListWidget { background-color: #f5f5f5; border: none; outline: none; }
        QListWidget::item { background-color: transparent; border: none; }
        QListWidget::item:hover { background-color: transparent; }
        QListWidget::item:selected { background-color: transparent; }
        QScrollBar:vertical { border: none; background: transparent; width: 8px; margin: 0px; }
        QScrollBar::handle:vertical { background: #d0d0d0; min-height: 20px; border-radius: 4px; }
        QScrollBar::handle:vertical:hover { background: #a0a0a0; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }
    )");

    this->setResizeMode(QListWidget::Adjust);


    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 到底部加载历史的信号
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this,
        [this](int value) {
            if (value == 0 && this->count() > 0) {
                emit sigLoadingHistory();
            }
        });

    // 滚动动画初始化
    m_scrollAnimation = new QPropertyAnimation(this->verticalScrollBar(), "value", this);
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_scrollAnimation->setDuration(250);
}

void NChatView::setSessionInfo(const SessionInfo& info)
{
    this->clear(); // 清空旧消息
    m_currentOtherAvatar = info.avatarPath(); // 记录对方头像
    m_lastMsgTime = QDateTime(); // 重置时间记录

    const QList<Message>& msgs = info.messages();

    for (const Message& msg : msgs) {

        if (!m_lastMsgTime.isValid() || m_lastMsgTime.secsTo(msg.timestamp) > 300) {
            addTimeItem(msg.timestamp);
        }

        QString avatar = msg.isSelf ? "../../data/avatar/me.jpg" : m_currentOtherAvatar;

        appendMessage(msg, avatar);

        // 更新最后一条消息的时间
        m_lastMsgTime = msg.timestamp;
    }

    this->scrollToBottom();
}

void NChatView::appendMessage(const Message& msg, const QString& avatarPath)
{
    NChatMessageItem* widget = new NChatMessageItem(msg.isSelf, msg.content, avatarPath);

    int viewportWidth = this->viewport()->width();
    int itemRealWidth = viewportWidth - 30;
    if (itemRealWidth < 200) itemRealWidth = 200;

    widget->adjustContent(itemRealWidth);

    QListWidgetItem* item = new QListWidgetItem(this);

    item->setSizeHint(widget->sizeHint());

    this->setItemWidget(item, widget);
}

void NChatView::addTimeItem(const QDateTime& time)
{
    QWidget* timeWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(timeWidget);

    QLabel* lblTime = new QLabel(time.toString("MM-dd HH:mm"));

    lblTime->setStyleSheet("color: #cfcfcf; font-size: 12px; padding: 4px; border-radius: 4px;");

    layout->addStretch();
    layout->addWidget(lblTime);
    layout->addStretch();

    QListWidgetItem* item = new QListWidgetItem(this);
    item->setSizeHint(QSize(0, 40)); // 时间条高度较小且固定
    item->setFlags(Qt::NoItemFlags); // 时间条通常不可选中

    this->setItemWidget(item, timeWidget);
}

void NChatView::resizeEvent(QResizeEvent* event)
{
    QListWidget::resizeEvent(event);

    int w = this->viewport()->width() - 30; // 同样保持预留
    if (w < 200) w = 200;

    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        NChatMessageItem* widget = qobject_cast<NChatMessageItem*>(this->itemWidget(item));

        if (widget) {

            widget->adjustContent(w);

            item->setSizeHint(widget->sizeHint());
        }
    }
}

void NChatView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QListWidget::wheelEvent(event);
        return;
    }
    m_scrollAnimation->stop();
    int currentValue = this->verticalScrollBar()->value();
    int delta = event->angleDelta().y();
    int step = 150;

    m_targetScrollValue = (delta > 0) ? currentValue - step : currentValue + step;

    if (m_targetScrollValue < 0) m_targetScrollValue = 0;
    if (m_targetScrollValue > this->verticalScrollBar()->maximum())
        m_targetScrollValue = this->verticalScrollBar()->maximum();

    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(m_targetScrollValue);
    m_scrollAnimation->start();
    event->accept();
}
