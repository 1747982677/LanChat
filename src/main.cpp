#include <QtWidgets/QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "utils/config.h"
#include "utils/db_manager.h"
#include "utils/logger.h"
#include "ui/main_window/main_window.h"
#include "ui/db_qwidget/DbQWidget.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 初始化日志
   /* Logger::getInstance().init("lanchat.log");
    Logger::getInstance().warning("111");*/

    //// 加载配置
    //Config::getInstance().load("config.ini");

    //// 从配置读取数据库路径（可为空，DatabaseManager 会使用默认路径）
    //QString dbPath = Config::getInstance().getString("database/path", QString());

    // 初始化数据库（示例：注意反斜杠需要转义）
    if (!DatabaseManager::getInstance().init(QStringLiteral("C:\\mty\\LanChat\\src\\model\\lanchat.db"))) {
        //Logger::getInstance().error("");
        qDebug().noquote() << "Failed to initialize database";
    }
    else {
        //Logger::getInstance().log("Database initialized");
        qDebug().noquote() << "Database initialized";
    }
    
   /* QWidget window;
    window.setWindowTitle("LanChat");
    window.resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(&window);
    QLabel* label = new QLabel("LanChat is working!", &window);
    label->setAlignment(Qt::AlignCenter);
    
    QFont font = label->font();
    font.setPointSize(14);
    font.setBold(true);
    label->setFont(font);
    
    layout->addWidget(label);
    
    window.show();*/

    // 创建并显示主窗口（MainWindow 会在构造中 setupUi）
    DbQWidget dbQWidget;
    dbQWidget.show();

    
    // 关闭主线程 DB 连接
    DatabaseManager::getInstance().closeConnectionForCurrentThread();
    //Logger::getInstance().close();
    return app.exec();
}
