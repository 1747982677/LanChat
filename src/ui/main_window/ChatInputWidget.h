#ifndef CHATINPUTWIDGET_H
#define CHATINPUTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>

class ChatInputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatInputWidget(QWidget* parent = nullptr);

signals:
    void signalSendMessage(const QString& text);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSendClicked();

private:
    void initUI();
    void initStyle();

    QTextEdit* m_textEdit;      // 输入框
    QPushButton* m_btnSend;     // 发送按钮
};

#endif // CHATINPUTWIDGET_H
