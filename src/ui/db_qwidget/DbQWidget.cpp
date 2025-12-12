#include "DbQWidget.h"
#include "utils/logger.h"
#include <QMessageBox>
#include "utils/db_manager.h"
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include "model/message_dao.h"
#include <QtTest/QtTest>
#include "utils/database.h"
#include "core/app_context.h"

DbQWidget::DbQWidget(QWidget *parent)
	: QWidget(parent)
{
	uii.setupUi(this);
    initUI();
    connectSignals();
    Logger::getInstance().log("MainWindow created");
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
    bool connected = connect(dbCtrl, &DbLogicController::queryResultsReady,
        this, &DbQWidget::queryResultsReady);
    Logger::getInstance().log(QString("[MainWindow] Signal connection result: %1").arg(connected ? "SUCCESS" : "FAILED"));

    if (connected) {
        Logger::getInstance().log("[MainWindow] Signal connected to DbLogicController");
    }
    else {
        Logger::getInstance().error("[MainWindow] FAILED to connect signal!");
    }
}

DbQWidget::~DbQWidget()
{
    Logger::getInstance().log("DbQWidget destroyed");
}

void DbQWidget::initUI()
{
    // Set window title
    setWindowTitle("LanChat - This is Db TestTool");
    // TODO: Initialize UI components
}

void DbQWidget::connectSignals()
{
    // TODO: Connect signals and slots
    // 直接使用 Designer 生成的成员名 pushButtonConnect
    connect(uii.testDbBut, &QPushButton::clicked, this, &DbQWidget::onConnectClicked);
    connect(uii.msgAddBut, &QPushButton::clicked, this, &DbQWidget::addMessages);
    connect(uii.msgDelBut, &QPushButton::clicked, this, &DbQWidget::delMessages);
    connect(uii.msgUpdateBut, &QPushButton::clicked, this, &DbQWidget::updateMessages);
    connect(uii.msgSearchBut, &QPushButton::clicked, this, &DbQWidget::searchMessages);

    connect(uii.createTable, &QPushButton::clicked, this, &DbQWidget::createTable);
    connect(uii.addData, &QPushButton::clicked, this, &DbQWidget::addData);
    connect(uii.delData, &QPushButton::clicked, this, &DbQWidget::delData);
    connect(uii.updateData, &QPushButton::clicked, this, &DbQWidget::updateData);
    connect(uii.searchData, &QPushButton::clicked, this, &DbQWidget::searchData);

    
    //connect(uii, &QPushButton::clicked, this, &DbQWidget::onConnectClicked);

}

void DbQWidget::addMessages() {
    // 测试插入消息(通过消息实体类)
    QString testMsg = "测试消息";
    Message m;
    m.sender = "alice";
    m.receiver = "bob";
    m.content = QString::fromUtf8(u8"测试消息");
    m.timestamp = QDateTime::currentDateTimeUtc();
    m.status = 0;
    m.extra = "";

    qint64 id = MessageDao::insertMessage(m);
    if (id > 0) 
        qDebug() << "新增记录ID:" << id;
    else
    qDebug() << "插入失败！";
}

void DbQWidget::delMessages() {
   
    bool flag = MessageDao::delMsg(3);
    if (flag)
        qDebug() << "删除成功！" ;
    else
        qDebug() << "删除失败！";
}


void DbQWidget::updateMessages() {
    // 测试插入消息
    QString testMsg = "测试消息";
    Message m;
    m.sender = "alice";
    m.receiver = "bob";
    m.content = QString::fromUtf8(u8"测试消息");
    m.timestamp = QDateTime::currentDateTimeUtc();
    m.status = 0;
    m.extra = "";
    qint64 id = MessageDao::insertMessage(m);
   
    bool insertFlag= id > 0;
    qDebug() << "插入标识:" << insertFlag;
    bool flag = MessageDao::updateStatus(1, 1);
    if (flag) {
        uii.listWidget->clear();
        uii.listWidget->addItem("状态更新成功");
    }
}

void DbQWidget::queryResultsReady(const QVector<Message>& conv) {
    uii.listWidget->clear();
    for (const Message& msg : conv) {
        QString itemText = QString("[%1] %2: %3")
            .arg(msg.timestamp.toString())
            .arg(msg.sender)
            .arg(msg.content);
        uii.listWidget->addItem(itemText);
    }
}
void DbQWidget::searchMessages() {
    DbLogicController* dbCtrl = AppContext::instance().dbLogicController();
 
    //根据sender和receiver获取最多limit数量的消息记录
    //QVector<Message> conv = MessageDao::getConversation("alice", "bob", 100);

    dbCtrl->requestQueryMessages("alice", "bob", 100);
    /*QCOMPARE(conv.size(), 1);
    QCOMPARE(conv.first().content, m.content);*/

   
}

void DbQWidget::createTable()
{
    // ===================== 1. 创建 Sqlite 对象 =====================
    // 参数说明：数据库文件名（本地路径）、主机名（SQLite 无需，传""）、用户名（""）、密码（""）
    // 数据库文件会自动生成在程序运行目录下（如：test.db）
    //
    Sqlite db("C:\\mty\\LanChat\\src\\model\\public.db", "", "", "");

    //// ===================== 2. 创建数据表（示例：用户表 user） =====================
    //// 定义字段名（key）和字段类型（value）：id(主键自增)、name(文本)、age(整数)、score(实数)
    QVector<QString> tableKeys = { "userid", "username", "age", "hobby","gender" };
    QVector<QString> tableValues = {
        "INTEGER PRIMARY KEY AUTOINCREMENT",  // id 自增主键
        "TEXT NOT NULL",                      // name 非空文本
        "INTEGER DEFAULT 0",                   // age 默认0
        "TEXT",                                 // 
        "TEXT NOT NULL",                      // name 非空文本
    };

    // 创建表（若表已存在，会返回 false，可通过 IsTableExist 先判断）
    if (db.IsTableExist("user")) {
        qDebug() << "表 user 已存在，无需重复创建";
    }
    else {
        bool createOk = db.create_table("user", tableKeys, tableValues);
        if (createOk) {
            qDebug() << "表 user 创建成功";
        }
        else {
            qDebug() << "表创建失败：" << db.getError();
        }
    }
}

void DbQWidget::addData()
{
    Sqlite db("C:\\mty\\LanChat\\src\\model\\public.db", "", "", "");

    // ===================== 3. 插入数据（add 函数） =====================
    QMap<QString, QString> addData;
    addData["username"] = "'张三'";    // 文本类型需加单引号（SQL 语法要求）
    addData["age"] = "18";         // 数字类型可直接传字符串
    addData["hobby"] = "'羽毛球'";    // 文本类型需加单引号（SQL 语法要求）
    addData["gender"] = "'女'";    // 文本类型需加单引号（SQL 语法要求）

    bool addOk = db.add("user", addData);
    if (addOk) {
        qDebug() << "插入数据成功，当前表行数：" << db.get_rowCount("user");
    }
    else {
        qDebug() << "插入失败：" << db.getError();
    }

    // 再插入一条数据
    addData["username"] = "'王五'";    // 文本类型需加单引号（SQL 语法要求）
    addData["age"] = "20";         // 数字类型可直接传字符串
    addData["hobby"] = "'乒乓球'";    // 文本类型需加单引号（SQL 语法要求）
    addData["gender"] = "'男'";    // 文本类型需加单引号（SQL 语法要求）
    db.add("user", addData);
}

void DbQWidget::delData()
{
    Sqlite db("C:\\mty\\LanChat\\src\\model\\public.db", "", "", "");
    //// ===================== 6. 删除数据（del 函数） =====================
    QMap<QString, QString> delWhere;
    delWhere["username"] = "'李四'"; // 条件：删除姓名=李四的记录
    QList<QString> queryKeys = { "userid", "username", "age", "hobby","gender"}; // 要查询的字段
    QList<QList<QString>> allRows;

    bool delOk = db.del("user", delWhere);
    if (delOk) {
        qDebug() << "\n删除数据成功，当前表行数：" << db.get_rowCount("user");
        // 重新查询所有数据
        db.find("user", queryKeys, &allRows);
        qDebug() << "删除后剩余数据：";
        for (const auto& row : allRows) {
            qDebug() << "userid:" << row[0] << " 姓名:" << row[1] << " 年龄:" << row[2] << " 爱好:" << row[3] << " 性别:" << row[4];
        }
    }
    else {
        qDebug() << "删除失败：" << db.getError();
    }
}

void DbQWidget::updateData()
{
    Sqlite db("C:\\mty\\LanChat\\src\\model\\public.db", "", "", "");

    //// ===================== 5. 更新数据（updata 函数，注意拼写：updata 少了个 'e'） =====================
    QMap<QString, QString> updateData;
    updateData["hobby"] = "'游泳'"; // 要更新的字段：爱好改为游泳
    QMap<QString, QString> updateWhere;
    updateWhere["username"] = "'张三'"; // 条件：name=张三

    bool updateOk = db.updata("user", updateWhere, updateData); // 注意函数名是 updata（非 update）

    QList<QString> queryKeys = { "userid", "username", "age", "hobby","gender" }; // 要查询的字段
    QList<QList<QString>> condRows;
    if (updateOk) {
        qDebug() << "\n更新数据成功";
        // 重新查询验证
        db.find("user", queryKeys, updateWhere, &condRows);
        qDebug() << "张三更新后的爱好：" << condRows[0][3];
    }
    else {
        qDebug() << "更新失败：" << db.getError();
    }
}

void DbQWidget::searchData()
{
    Sqlite db("C:\\mty\\LanChat\\src\\model\\public.db", "", "", "");

    //// ===================== 4. 查询数据（3种 find 重载，示例常用2种） =====================
    // 4.1 查询所有数据（无条件）
    QList<QList<QString>> allRows;
    QList<QString> queryKeys = { "userid", "username", "age", "hobby","gender" }; // 要查询的字段
    bool findAllOk = db.find("user", queryKeys, &allRows);
    if (findAllOk) {
        qDebug() << "\n查询所有数据（共" << allRows.size() << "条）：";
        for (const auto& row : allRows) {
            qDebug() << "userid:" << row[0] << " 姓名:" << row[1] << " 年龄:" << row[2] << " 爱好:" << row[3] << " 性别:" << row[4];
        }
    }

    // 4.2 条件查询（比如：查询 age=20 的用户）
    QMap<QString, QString> whereCond;
    whereCond["age"] = "20"; // 条件：age=20
    QList<QList<QString>> condRows;
    bool findCondOk = db.find("user", queryKeys, whereCond, &condRows);
    if (findCondOk) {
        qDebug() << "\n条件查询（age=20）结果：";
        for (const auto& row : condRows) {
            qDebug() << "id:" << row[0] << " 姓名:" << row[1] << " 年龄:" << row[2] << " 爱好:" << row[3] << " 性别:" << row[4];
        }
    }
}

void DbQWidget::onConnectClicked()
{
    Logger::getInstance().log("Connect button clicked");

    QSqlDatabase db = DatabaseManager::getInstance().database();
    if (!db.isValid() || !db.isOpen()) {
        QString err = db.isValid() ? db.lastError().text() : QString("Invalid database handle");
        Logger::getInstance().error(QString("Database not open: %1").arg(err));
        QMessageBox::critical(this, tr("数据库连接测试"), tr("数据库未打开: %1").arg(err));
        return;
    }

    QSqlQuery q(db);
    if (!q.exec("SELECT 1")) {
        QString err = q.lastError().text();
        Logger::getInstance().error(QString("Test query failed: %1").arg(err));
        QMessageBox::critical(this, tr("数据库连接测试"), tr("连接测试失败: %1").arg(err));
        return;
    }

    Logger::getInstance().log("Database test query succeeded");
	uii.lineEdit->setText(QStringLiteral("数据库连接测试成功"));

}

