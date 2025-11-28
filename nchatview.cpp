#include "nchatview.h"
#include "nchatmessageitem.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollBar>

NChatView::NChatView(QWidget *parent) : QListWidget(parent)
{
    // 去掉边框和背景色，让它融入窗口
    this->setFrameShape(QFrame::NoFrame);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // 平滑滚动
    this->setSelectionMode(QAbstractItemView::NoSelection); // 聊天列表通常不需要选中整行

    this->setStyleSheet(R"(
        /* --- 列表整体设置 --- */
        QListWidget {
            background-color: #f5f5f5;
            border: none;
            outline: none; /* 去掉选中时的虚线框 */
        }

        /*--- 悬停和选中 ---*/
        QListWidget::item {
            background-color: transparent;
            border: none;
        }
        QListWidget::item:hover {
            background-color: transparent;
        }
        QListWidget::item:selected {
            background-color: transparent;
        }

        /* --- 滚动条 --- */
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 8px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #d0d0d0;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #a0a0a0;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

    this->setResizeMode(QListWidget::Adjust);

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this](int value){
                // 如果滚动条到了顶部 (value == 0) 且当前确实有消息
                if (value == 0 && this->count() > 0) {
                    emit sigLoadingHistory();
                }
            });

    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 初始化滚动动画
    m_scrollAnimation = new QPropertyAnimation(this->verticalScrollBar(), "value", this);
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad); // 缓出曲线，感觉更自然
    m_scrollAnimation->setDuration(250); // 动画时长 250ms
}

void NChatView::addMessage(const QString &text, MsgRole role, const QString &avatarPath)
{
    NChatMessageItem *messageWidget = new NChatMessageItem(role, text, avatarPath);
    QListWidgetItem *item = new QListWidgetItem(this);
    item->setSizeHint(messageWidget->sizeHint());
    this->setItemWidget(item, messageWidget);
    scrollToBottom();
}

void NChatView::addTime(const QString &timeStr)
{
    // 时间戳比较简单，直接用一个居中的 QLabel 即可，不需要复杂的 Widget 类
    QWidget *timeWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(timeWidget);
    QLabel *lblTime = new QLabel(timeStr);

    lblTime->setStyleSheet("color: #999999; font-size: 12px; padding: 4px;");
    layout->addStretch();
    layout->addWidget(lblTime);
    layout->addStretch();

    QListWidgetItem *item = new QListWidgetItem(this);
    item->setSizeHint(QSize(0, 40)); // 时间条高度固定
    this->setItemWidget(item, timeWidget);
}


// 到底部追加
void NChatView::appendMessage(const MessageData &msg) {
    NChatMessageItem *widget = new NChatMessageItem(msg.role(), msg.content(), msg.avatarurl());
    QListWidgetItem *item = new QListWidgetItem(this);
    item->setSizeHint(widget->sizeHint());
    this->setItemWidget(item, widget);
    this->scrollToBottom();
}
// 到顶部插入
void NChatView::insertMessageFront(const MessageData &msg) {
    NChatMessageItem *widget = new NChatMessageItem(msg.role(), msg.content(), msg.avatarurl());
    QListWidgetItem *item = new QListWidgetItem(); // 创建悬空 item

    // 【关键】插入到第 0 行
    this->insertItem(0, item);

    item->setSizeHint(widget->sizeHint());
    this->setItemWidget(item, widget);
}

// 滚动动画
void NChatView::wheelEvent(QWheelEvent *event)
{
    // 如果按住了 Ctrl 或者是触摸板手势，可能需要交给父类处理（缩放等）
    if (event->modifiers() & Qt::ControlModifier) {
        QListWidget::wheelEvent(event);
        return;
    }

    m_scrollAnimation->stop();

    int currentValue = this->verticalScrollBar()->value();


    int delta = event->angleDelta().y();
    int step = 150;

    if (delta > 0) {
        m_targetScrollValue = currentValue - step;
    } else {
        m_targetScrollValue = currentValue + step;
    }

    if (m_targetScrollValue < 0) m_targetScrollValue = 0;
    if (m_targetScrollValue > this->verticalScrollBar()->maximum())
        m_targetScrollValue = this->verticalScrollBar()->maximum();

    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(m_targetScrollValue);

    m_scrollAnimation->start();

    event->accept();
}
