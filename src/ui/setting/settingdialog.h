#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingDialog; }
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget* parent = nullptr);
    ~SettingDialog();

private slots:
    // 清理聊天数据
    void on_btnClearStorage_clicked();

    // 退出账号按钮（pushButton_3）
    void on_pushButton_3_clicked();

    // 清空聊天记录操作完成回调
    void onChatHistoryCleared(bool success, const QString& errorMessage);

    // 统计 messages 表大小回调
    void onMessagesTableSizeCalculated(bool success, qint64 sizeBytes, const QString& errorMessage);

    // 日志级别下拉框
    void on_comboLogLevel_currentIndexChanged(int index);

private:
    // 初始化日志级别下拉框
    void initLogLevelUi();

private:
    Ui::SettingDialog* ui;
};

#endif // SETTINGDIALOG_H
