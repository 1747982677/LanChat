#ifndef NETWORK_ERROR_DIALOG_H
#define NETWORK_ERROR_DIALOG_H

#include <QDialog>

namespace Ui {
    class NetworkErrorDialog;
}

/**
 * 网络错误提示弹窗
 * - 显示“网络连接错误”+ 图标
 * - 底部“确认”按钮，点击后关闭弹窗
 */
class NetworkErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkErrorDialog(QWidget* parent = nullptr);
    ~NetworkErrorDialog();

    // 可选：在代码里动态修改副标题提示
    void setSubtitleText(const QString& text);

protected:
    // 用来在 show 时把窗口居中（可去掉）
    void showEvent(QShowEvent* event) override;

private slots:
    // ui 里按钮叫 pushButton，自动绑定 on_pushButton_clicked
    void on_pushButton_clicked();

private:
    Ui::NetworkErrorDialog* ui;
};

#endif // NETWORK_ERROR_DIALOG_H
