#include "SessionListItem.h"
#include "PixmapUtils.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>

SessionListItem::SessionListItem(QWidget* parent) : QWidget(parent)
{
    /*---- 列表项布局 ----*/
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(45, 45);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setStyleSheet("background-color: transparent;");

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");

    m_msgLabel = new QLabel(this);
    m_msgLabel->setStyleSheet("font-size: 12px; color: #999;");
    m_msgLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    QVBoxLayout* textVLayout = new QVBoxLayout;
    textVLayout->setContentsMargins(0, 5, 0, 5);
    textVLayout->setSpacing(5);
    textVLayout->addWidget(m_titleLabel);
    textVLayout->addWidget(m_msgLabel);

    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet(
        "font-size: 11px; color: #b2b2b2;"
    );
    m_timeLabel->setAlignment(Qt::AlignRight);
    m_timeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_unReadNum = new QLabel(this);
    m_unReadNum->setFixedHeight(18);
    m_unReadNum->setMinimumWidth(18);
    m_unReadNum->setAlignment(Qt::AlignCenter);

    m_unReadNum->setObjectName("unreadBadge");
    m_unReadNum->setStyleSheet(
        "QLabel#unreadBadge {"
        "   background-color: #FF3B30;"
        "   color: white;"
        "   font-size: 10px;"
        "   border-radius: 9px;"
        "   padding: 0 4px;"
        "}"
    );
    m_unReadNum->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_unReadNum->hide();


    QVBoxLayout* rightVLayout = new QVBoxLayout;
    rightVLayout->setContentsMargins(0, 5, 0, 5);
    rightVLayout->setSpacing(5);
    rightVLayout->addWidget(m_timeLabel, 0, Qt::AlignRight);
    rightVLayout->addWidget(m_unReadNum, 0, Qt::AlignRight);
    rightVLayout->addStretch();


    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(10);

    mainLayout->addWidget(m_avatarLabel, 0);
    mainLayout->addLayout(textVLayout, 1);
    mainLayout->addLayout(rightVLayout, 0);

    m_avatarLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_msgLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_timeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_unReadNum->setAttribute(Qt::WA_TransparentForMouseEvents);
}


void SessionListItem::setData(const SessionInfo& data)
{

    m_data = data;  // 保存数据

    m_avatarLabel->setPixmap(PixmapUtils::clipToCircle(data.avatarPath(), 45));
    m_titleLabel->setText(data.username());
    m_timeLabel->setText(formatTime(data.lastTime()));

    if (data.unreadCount() > 0) {
        m_unReadNum->show();

        int count = data.unreadCount();
        if (count > 99) {
            m_unReadNum->setText("99+"); // 字数多了，背景会自动变宽
        }
        else {
            m_unReadNum->setText(QString::number(count));
        }
        m_unReadNum->adjustSize();
    }
    else {
        m_unReadNum->hide();
    }
    m_msgLabel->setText(data.lastMessage());

    updateMessageLabel();
}

void SessionListItem::updateMessageLabel() {
    int w = m_msgLabel->width();
    if (w <= 0) w = 200; // 默认防守值

    QFontMetrics metrics(m_msgLabel->font());
    QString elidedText = metrics.elidedText(m_data.lastMessage(), Qt::ElideRight, w);
    m_msgLabel->setText(elidedText);
}

void SessionListItem::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateMessageLabel(); 
}

void SessionListItem::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    // 只有置顶时才绘制特殊背景
    if (m_data.isTop()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
        painter.setPen(Qt::NoPen); // 不要边框

        painter.setBrush(QColor(0, 0, 0, 15));

        painter.drawRoundedRect(this->rect(), 8, 8);
    }
}

QString SessionListItem::elideText(const QString& text, int width)
{
    QFontMetrics metrics(m_msgLabel->font());
    return metrics.elidedText(text, Qt::ElideRight, width);
}


QString SessionListItem::formatTime(const QDateTime& dt)
{
    if (!dt.isValid()) return "";

    QDateTime now = QDateTime::currentDateTime();
    if (dt.date() == now.date()) {
        return dt.toString("HH:mm");
    }
    else if (dt.daysTo(now) == 1) {
        return "昨天";
    }
    else if (dt.daysTo(now) < 7) {
        return dt.toString("dddd");
    }
    else {
        return dt.toString("yy/MM/dd");
    }
}