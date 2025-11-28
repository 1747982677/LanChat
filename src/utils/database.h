#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMap>
#include <QInternal>
#include <QString>
#include <QDebug>
#include <QMutex>
class Sqlite
{
public:
    QSqlDatabase db;
    QString host_name;  //主机名
    QString user_name;  //用户名
    QString pwd;    //密码
    QString db_file_name;   //数据库文件名
    QSqlError error;
    QSqlQuery* query;
    QSqlRecord rec;
public:
    Sqlite(QString m_file_name, QString m_host_name, QString m_user_name, QString m_pwd);
    ~Sqlite();
    QSqlDatabase getDatabase();
    bool create_table(QString table_name, QVector<QString> key, QVector<QString> value);   //新建表
    int get_rowCount(QString table_name);
    bool db_query(QString m_query_sql); //发送数据库语句
    bool add(QString table_name, QMap<QString, QString> data); //增加数据
    bool del(QString table_name, QMap<QString, QString> where); //删除一条记录
    bool updata(QString table_name, QMap<QString, QString> where, QMap<QString, QString> data);  //更新数据
    bool find(QString table_name, QList<QString> key, QMap<QString, QString> where, QList<QList<QString>>* row);    //查找
    bool find(QString table_name, QList<QString> key, QString WhereKey, std::map<QString, std::vector<QString>>& row);    //查找
    bool find(QString table_name, QList<QString> key, QList<QList<QString>>* row);    //查找所有
    bool IsTableExist(QString table);  //查询表是否存在
    int Get_sqlCount(QString sqlstr);
    QString getError();
    QMutex mutex;
};

#endif // DATABASE_H