#include "network_error_dialog.h"
#include "ui_network_error_dialog.h"   // ? 一定是这个名字，对应 network_error_dialog.ui

#include <QShowEvent>
#include <QApplication>
#include <QScreen>

NetworkErrorDialog::NetworkErrorDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NetworkErrorDialog)
{
    ui->setupUi(this);

    // 去掉 ? 帮助按钮（可选）
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 标题加粗一点（你也可以直接在 .ui 里设）
    ui->lblTitle->setStyleSheet("font-weight:600;");

    // 图标保持缩放填充
    ui->lblBigIcon->setScaledContents(true);
}

NetworkErrorDialog::~NetworkErrorDialog()
{
    delete ui;
}

void NetworkErrorDialog::setSubtitleText(const QString& text)
{
    ui->lblSubtitle->setText(text);
}

void NetworkErrorDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    // 简单居中到父窗口 / 屏幕中心
    QWidget* p = parentWidget();
    if (p) {
        const QPoint c = p->geometry().center();
        move(c.x() - width() / 2, c.y() - height() / 2);
    }
    else {
        const QRect rect = QApplication::primaryScreen()->availableGeometry();
        move(rect.center().x() - width() / 2,
            rect.center().y() - height() / 2);
    }
}

// 点击“确认”按钮：关闭弹窗
void NetworkErrorDialog::on_pushButton_clicked()
{
    accept();   // 如果是 exec() 打开的，会返回 QDialog::Accepted
}
