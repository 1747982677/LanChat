#include "settingdialog.h"
#include "ui_settingdialog.h"
#include "ui/main_window/main_window.h" // 两个分支都有，只保留一个
#include "ui/db_qwidget/DbQWidget.h"
#include "ui/login/login_window.h"
#include "service/auth_service.h"
#include "utils/logger.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDebug>

SettingDialog::SettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

// 「更改存储路径」按钮
void SettingDialog::on_btnChangeStoragePath_clicked()
{
    // 以当前显示的路径作为初始目录；如果为空就用用户家目录
    QString currentPath = ui->lblStoragePath->text().trimmed();
    if (currentPath.isEmpty()) {
        currentPath = QDir::homePath();
    }

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择聊天数据保存路径"),
        currentPath
    );

    if (!dir.isEmpty()) {
        ui->lblStoragePath->setText(dir);
        qDebug() << "[SettingDialog] Storage path changed to:" << dir;
        // TODO: 在这里调用配置模块，真正把路径写入配置文件
        // 例如：Config::instance().setStoragePath(dir);
    }
}

// 「清理」按钮
void SettingDialog::on_btnClearStorage_clicked()
{
    const QString path = ui->lblStoragePath->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(
            this,
            "清理聊天数据",
            "当前存储路径为空，无法清理。"
        );
        return;
    }

    // 二次确认
    QMessageBox::StandardButton ret = QMessageBox::question(
        this,
        "清理聊天数据",
        QString("确定要清理以下路径下的所有聊天数据吗？\n%1").arg(path),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (ret != QMessageBox::Yes) {
        qDebug() << "[SettingDialog] Clear storage canceled by user";
        return;
    }

    // TODO: 在这里调用实际的清理逻辑（删除数据库 / 文件 / 缓存等）
    // 例如：
    // DbLogicController::instance().clearChatHistory(path);

    qDebug() << "[SettingDialog] Request to clear storage at:" << path;

    QMessageBox::information(
        this,
        "清理聊天数据",
        "已发起清理请求（具体清理逻辑待实现）。"
    );
}
void logoutAndShowLogin()
{
    // 1. 清除登录状态（token 等）
    AuthService& authService = AuthService::getInstance();
    authService.logout();    // 需要你在 AuthService 里实现这个函数，清 token / 标记未登录

    // 2. 设置用户状态为离线
    MainWindow* mainWindow = MainWindow::instance();
    mainWindow->updateUserStatus(false);
    
    // 3. 隐藏主窗口
    mainWindow->hide();

    // 3. 创建登录窗口，按原来的方式连接 loginSucceeded
    LoginWindow* loginWindow = new LoginWindow();

    QObject::connect(loginWindow, &LoginWindow::loginSucceeded,
        [loginWindow]() {
            qDebug() << "logout flow: 收到 loginSucceeded 信号";
            // 登录成功后，隐藏登录窗口
            loginWindow->hide();
            
            // 从 AuthService 获取当前登录的用户ID
            AuthService& authService = AuthService::getInstance();
            QString userId = authService.getCurrentUserId();
            
            if (userId.isEmpty()) {
                Logger::getInstance().error("logout flow: 错误：登录成功但 userId 为空");
                qDebug() << "logout flow: 错误：登录成功但 userId 为空";
                loginWindow->deleteLater();
                return;
            }
            
            // 设置新的用户ID并重新查询用户信息和加载联系人列表
            MainWindow* mainWindow = MainWindow::instance();
            mainWindow->userid = userId;
            Logger::getInstance().log("logout flow: 登录成功，重新加载用户数据，userId: " + userId);
            
            // 重新查询用户信息（这会触发联系人列表的重新加载）
            // queryUserReady 信号会调用 loadContacts()，而 loadContacts() 在加载新数据前会先清空旧数据
            mainWindow->requestQueryUser();
            mainWindow->show();
            loginWindow->deleteLater();
        });

    // 4. 显示登录窗口
    loginWindow->show();
}
// 「退出账号」按钮（对象名：pushButton_3）
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

    // 调用刚才写的工具函数：退出并切回登录页
    logoutAndShowLogin();

    // 关闭设置对话框本身
    accept();
}
