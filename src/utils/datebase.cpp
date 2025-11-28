#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMap>
#include <QInternal>
#include <QString>
#include <QDebug>
#include <QMutex>

Sqlite::Sqlite(QString m_file_name, QString m_host_name, QString m_user_name, QString m_pwd)
{
    host_name = m_host_name;
    user_name = m_user_name;
    pwd = m_pwd;
    db_file_name = m_file_name;
    db = QSqlDatabase::addDatabase("QSQLITE");
    query = new QSqlQuery(db);
    db.setHostName(host_name);
    db.setUserName(user_name);
    db.setPassword(pwd);
    db.setDatabaseName(db_file_name);
    if (!db.open())
    {
        //qDebug()<<"database open error.";
        error = db.lastError();
    }
}

Sqlite::~Sqlite()
{
    db.close();
}



QSqlDatabase Sqlite::getDatabase()
{
    return db;
}

/*
 *
 * create_table函数：创建数据库表
 * table_name:表名
 * table_data:表项 名->属性
 *
 */
bool Sqlite::create_table(QString table_name, QVector<QString> key, QVector<QString> value)
{
    QMutexLocker locker(&mutex);
    QString sql = "create table if not exists " + table_name + " (";
    for (int i = 0; i < key.count(); i++)
    {
        sql += key[i] + ' ' + value[i] + ',';
    }
    sql.chop(1);
    sql += ")";
    query->prepare(sql);
    return query->exec();
}

int Sqlite::get_rowCount(QString table_name)
{
    QMutexLocker locker(&mutex);
    QSqlQuery query;
    QString sql = "SELECT count(*) FROM " + table_name;
    query.prepare(sql);
    if (query.exec())
    {
        query.next();
        return query.value(0).toInt();
    }
    else return 0;
}

/*
 * 向数据库中增加数据
 */
bool Sqlite::add(QString table_name, QMap<QString, QString> data)
{
    QMutexLocker locker(&mutex);
    QString sql = "insert into " + table_name + "(";
    QString values = " values(";
    for (QMap<QString, QString>::const_iterator i = data.constBegin(); i != data.constEnd(); i++)
    {
        sql += "" + i.key() + ", ";
        values += "" + i.value() + ", ";
    }
    sql.chop(2);
    values.chop(2);
    sql += ")";
    values += ")";
    sql += values;
    query->prepare(sql);
    return query->exec();
}

/*
 * 向数据库发送一条语句
 */
bool Sqlite::db_query(QString m_query_sql)
{
    QMutexLocker locker(&mutex);
    query->prepare(m_query_sql);
    return query->exec();
}

/*
 * 删除一条记录
 */
bool Sqlite::del(QString table_name, QMap<QString, QString> where)
{
    QMutexLocker locker(&mutex);
    QString sql = "delete from ";
    sql += table_name;
    sql += " where ";
    for (QMap<QString, QString>::const_iterator i = where.constBegin(); i != where.constEnd(); i++)
    {
        sql += i.key() + "=";
        sql += "" + i.value() + " ";
    }
    sql.chop(1);
    query->prepare(sql);
    return query->exec();
}

/*
 * 修改数据库记录
 */
bool Sqlite::updata(QString table_name, QMap<QString, QString> where, QMap<QString, QString> data)
{
    QMutexLocker locker(&mutex);
    QString sql = "update " + table_name + " set ";
    for (QMap<QString, QString>::const_iterator i = data.constBegin(); i != data.constEnd(); i++)
    {
        sql += i.key() + "=";
        sql += i.value() + " ";
    }
    sql += "where ";
    for (QMap<QString, QString>::const_iterator i = where.constBegin(); i != where.constEnd(); i++)
    {
        sql += i.key() + "=";
        sql += i.value() + " ";
    }
    query->prepare(sql);
    return query->exec();
}

/*
 * 查找所有记录
 */
bool Sqlite::find(QString table_name, QList<QString> key, QList<QList<QString>>* row)
{
    QMutexLocker locker(&mutex);
    QString sql = "select ";
    int len = key.size();
    for (int i = 0; i < len; i++)
    {
        sql += key.at(i);
        sql += ",";
    }
    sql.chop(1);
    sql += " from " + table_name;
    //qDebug()<<sql;
    query->prepare(sql);
    if (query->exec())
    {
        while (query->next()) {
            QList<QString> j;
            for (int i = 0; i < len; i++)
            {
                j.append(query->value(i).toString());
            }
            row->append(j);
        }
        return 1;
    }
    else return 0;
}

bool Sqlite::IsTableExist(QString table)
{
    QMutexLocker locker(&mutex);
    QString sql = QString("select count(*) from sqlite_master where type = 'table' name = '%1'").arg(table);
    if (db.tables().contains(table))
    {
        return true;
    }
    return false;
}

int Sqlite::Get_sqlCount(QString sqlstr)
{
    QMutexLocker locker(&mutex);
    query->prepare(sqlstr);
    if (query->exec())
    {
        query->next();
        return query->value(0).toInt();
    }
    else return 0;
}


bool Sqlite::find(QString table_name, QList<QString> key, QMap<QString, QString> where, QList<QList<QString> >* row)
{
    QMutexLocker locker(&mutex);
    QString sql = "select ";
    int len = key.size();
    for (int i = 0; i < len; i++)
    {
        sql += key.at(i);
        sql += ",";
    }
    sql.chop(1);
    sql += " from " + table_name;
    if (where.count())
    {
        sql += " where ";
        for (QMap<QString, QString>::const_iterator i = where.constBegin(); i != where.constEnd(); i++)
        {
            sql += i.key() + "=" + i.value() + ",";
        }
        sql.chop(1);
    }
    //qDebug()<<sql;
    query->prepare(sql);
    if (query->exec())
    {
        while (query->next()) {
            QList<QString> j;
            for (int i = 0; i < len; i++)
            {
                j.append(query->value(i).toString());
            }
            row->append(j);
        }
        return 1;
    }
    else return 0;
}



bool Sqlite::find(QString table_name, QList<QString> key, QString WhereKey, std::map<QString, std::vector<QString>>& row)
{
    QMutexLocker locker(&mutex);
    QString sql = "select ";
    int len = key.size();
    for (int i = 0; i < len; i++)
    {
        sql += key.at(i);
        sql += ",";
    }
    sql.chop(1);
    sql += " from " + table_name;
    sql += " where " + WhereKey;
    query->prepare(sql);
    if (query->exec())
    {
        while (query->next())
        {
            QList<QString> j;
            for (int i = 0; i < len; i++)
            {
                row[key.at(i)].push_back(query->value(i).toString());
            }
        }
        return 1;
    }
    else return 0;
}

/*
 * 获取错误信息
 */
QString Sqlite::getError()
{
    return error.text();
}
