#include "settingdialog.h"
#include "ui_settingdialog.h"

#include "ui/main_window/main_window.h"
#include "ui/login/login_window.h"
#include "service/auth_service.h"
#include "utils/logger.h"

#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include "core/dblogic_controller.h"

// 构造函数
SettingDialog::SettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    // 监听“清空所有聊天记录”操作的结果
    connect(&DbLogicController::instance(),
        &DbLogicController::allChatHistoryCleared,
        this,
        &SettingDialog::onChatHistoryCleared);

    // 监听 messages 表大小统计结果
    DbLogicController& dbCtrl = DbLogicController::instance();
    connect(&dbCtrl,
        &DbLogicController::messagesTableSizeCalculated,
        this,
        &SettingDialog::onMessagesTableSizeCalculated);

    // 打开设置页时先刷一次 messages 表大小
    dbCtrl.getMessagesTableSize();

    // 初始化日志级别下拉框
    initLogLevelUi();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

// -------------------- 工具函数：退出账号并回到登录页 --------------------
static void logoutAndShowLogin()
{
    // 1. 清除登录状态
    AuthService& authService = AuthService::getInstance();
    authService.logout();

    // 2. 设置用户状态为离线并隐藏主窗口
    MainWindow* mainWindow = MainWindow::instance();
    mainWindow->updateUserStatus(false);
    mainWindow->hide();

    // 3. 打开登录窗口
    LoginWindow* loginWindow = new LoginWindow();

    QObject::connect(loginWindow, &LoginWindow::loginSucceeded,
        [loginWindow]() {
            qDebug() << "logout flow: 收到 loginSucceeded 信号";

            loginWindow->hide();

            AuthService& authService = AuthService::getInstance();
            QString userId = authService.getCurrentUserId();

            if (userId.isEmpty()) {
                Logger::getInstance().error("logout flow: 错误：登录成功但 userId 为空");
                qDebug() << "logout flow: 错误：登录成功但 userId 为空";
                loginWindow->deleteLater();
                return;
            }

            MainWindow* mainWindow = MainWindow::instance();
            mainWindow->userid = userId;
            Logger::getInstance().log("logout flow: 登录成功，重新加载用户数据，userId: " + userId);

            // 重新查询用户信息并加载联系人列表
            mainWindow->requestQueryUser();
            mainWindow->show();

            loginWindow->deleteLater();
        });

    loginWindow->show();
}

// -------------------- 清理聊天数据按钮 --------------------
void SettingDialog::on_btnClearStorage_clicked()
{
    QMessageBox::StandardButton ret = QMessageBox::question(
        this,
        tr("清理聊天数据"),
        tr("确定要清理所有聊天数据吗？该操作无法恢复。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (ret != QMessageBox::Yes) {
        qDebug() << "[SettingDialog] Clear storage canceled by user";
        return;
    }

    // 只清空数据库中的聊天记录，不再管“路径”
    DbLogicController::instance().clearAllChatHistory();

    qDebug() << "[SettingDialog] Request to clear ALL chat history";
}

// -------------------- 清空聊天记录完成回调 --------------------
void SettingDialog::onChatHistoryCleared(bool success, const QString& errorMessage)
{
    if (success) {
        QMessageBox::information(
            this,
            tr("清理聊天数据"),
            tr("已成功清空所有聊天记录。")
        );
        // 再刷一遍 messages 表大小
        DbLogicController::instance().getMessagesTableSize();
    }
    else {
        QMessageBox::warning(
            this,
            tr("清理聊天数据"),
            tr("清空聊天记录失败：%1").arg(errorMessage)
        );
    }
}

// -------------------- 统计 messages 表大小回调 --------------------
void SettingDialog::onMessagesTableSizeCalculated(bool success,
    qint64 sizeBytes,
    const QString& errorMessage)
{
    if (!success) {
        ui->lblDataSize->setText(
            tr("无法统计（%1）").arg(errorMessage)
        );
        qDebug() << "[SettingDialog] calc size failed:" << errorMessage;
        return;
    }

    if (sizeBytes < 1024) {
        ui->lblDataSize->setText(tr("%1 B").arg(sizeBytes));
    }
    else {
        double sizeKB = sizeBytes / 1024.0;
        ui->lblDataSize->setText(
            tr("%1 KB").arg(QString::number(sizeKB, 'f', 2))
        );
    }

    qDebug() << "[SettingDialog] messages size =" << sizeBytes
        << "bytes, label =" << ui->lblDataSize->text();
}

// -------------------- 退出账号按钮 --------------------
void SettingDialog::on_pushButton_3_clicked()
{
    QMessageBox::StandardButton ret = QMessageBox::question(
        this,
        tr("退出账号"),
        tr("确定要退出当前账号吗？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (ret != QMessageBox::Yes) {
        qDebug() << "[SettingDialog] Logout canceled by user";
        return;
    }

    qDebug() << "[SettingDialog] Logout confirmed by user";

    logoutAndShowLogin();
    accept();
}

// -------------------- 日志级别初始化 --------------------
void SettingDialog::initLogLevelUi()
{
    QSettings settings("LanChat", "LanChatClient");

    int levelIndex = settings.value(
        "log/level",
        static_cast<int>(Logger::Level::Info)
    ).toInt();

    if (levelIndex < static_cast<int>(Logger::Level::Error) ||
        levelIndex > static_cast<int>(Logger::Level::Debug)) {
        levelIndex = static_cast<int>(Logger::Level::Info);
    }

    if (ui->comboLogLevel) {
        bool old = ui->comboLogLevel->blockSignals(true);
        ui->comboLogLevel->setCurrentIndex(levelIndex);
        ui->comboLogLevel->blockSignals(old);
    }

    Logger::getInstance().setLevel(static_cast<Logger::Level>(levelIndex));
}

// -------------------- 日志级别下拉框变更 --------------------
void SettingDialog::on_comboLogLevel_currentIndexChanged(int index)
{
    QSettings settings("LanChat", "LanChatClient");
    settings.setValue("log/level", index);

    Logger::Level level = Logger::Level::Info;

    switch (index) {
    case 0: level = Logger::Level::Error; break;
    case 1: level = Logger::Level::Warn;  break;
    case 2: level = Logger::Level::Info;  break;
    case 3: level = Logger::Level::Debug; break;
    default: level = Logger::Level::Info; break;
    }

    Logger::getInstance().setLevel(level);
    Logger::getInstance().log(QString("日志级别已修改，index=%1").arg(index));
}
