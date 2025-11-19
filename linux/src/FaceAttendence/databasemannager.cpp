#include "databasemannager.h"
#include <QDebug>
DatabaseManager* DatabaseManager::self = nullptr;
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{}

DatabaseManager *DatabaseManager::getInstance()
{
    if (!self)
        self = new DatabaseManager();
    return self;
}


bool DatabaseManager::initDataBase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("server.db");

    if (!db.open()) {
        qDebug() << db.lastError().text() << endl;
        return false;
    }

    QString createPersonInfoTable = R"(
                                    CREATE TABLE IF NOT EXISTS PersonInfoTable (
                                    info_id INTEGER   PRIMARY KEY AUTOINCREMENT,
                                    user_id TEXT UNIQUE,
                                    identity TEXT,
                                    work_id TEXT,
                                    name TEXT,
                                    face_file TEXT,
                                    dept TEXT
                                    )
                                    )";

    QString createVisitRecordTable = R"(
                                     CREATE TABLE IF NOT EXISTS VisitRecordTable (
                                     visit_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                     user_id TEXT,
                                     visit_time TIMESTAMP NOT NULL DEFAULT(datetime('now', 'localtime')),                              -- 来访时间
                                     FOREIGN KEY(user_id) REFERENCES PersonInfoTable(user_id)
                                     )
                                     )";

    QSqlQuery query;

    if (!query.exec(createPersonInfoTable)) {
        qDebug() << query.lastError().text() << endl;
        return false;
    }

    if (!query.exec(createVisitRecordTable)) {
        qDebug() << query.lastError().text() << endl;
        return false;
    }
    return true;
}

UserInfo DatabaseManager::getInfoByUID(const QString& user_id)
{

    UserInfo info{};
    // 1) 用当前的数据库连接构造一个 QSqlQuery 对象。
    //    这里的 db 应该是 DatabaseManager 的成员变量，并且已经 open()。
    QSqlQuery query(db);


    // 2) 预编译 SQL，使用占位符 :token 来替代实际值（防止 SQL 注入，也更高效）。
    query.prepare("SELECT name, work_id, identity, face_file, dept FROM PersonInfoTable WHERE user_id = :id");


    query.bindValue(":id", user_id);

    // 4) 执行 SQL.exec() 返回是否成功执行（语法、连接等问题会导致 false）。
    //    query.next() 向结果集移动到下一行（如果存在行会返回 true），
    //    因为 SELECT 只取 name 一列，query.value(0) 就是该列的值。
    if (query.exec() && query.next()) {
        // 5) 读取第一列（索引 0）的值并转换为 QString 返回。
        info.name      = query.value(0).toString();
        info.workId    = query.value(1).toString();
        info.identity  = query.value(2).toString();
        info.imgPath   = query.value(3).toString();
        info.dept      = query.value(4).toString();
        info.valid     = true;

        qDebug() << "name     from db:" << info.name      << endl;
        qDebug() << "workId   from db:" << info.workId    << endl;
        qDebug() << "identity from db:" << info.identity  << endl;
        qDebug() << "imgPath  from db:" << info.imgPath   << endl;
        qDebug() << "valid    from db:" << info.valid     << endl;
        qDebug() << "dept     from db:" << info.dept      << endl;

    }

    // 6) 如果 SQL 执行失败或没有结果，返回空字符串表示“未找到”。
    return info;
}


bool DatabaseManager::insertPersonInfo(QString user_id,
                                       QString identity,
                                       QString workId,
                                       QString name,
                                       QString faceFile,
                                       QString dept)
{
    QSqlQuery query;
    query.prepare("INSERT INTO PersonInfoTable "
                  "(user_id, identity, work_id, name, face_file, dept) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(user_id);
    query.addBindValue(identity);
    query.addBindValue(workId);
    query.addBindValue(name);
    query.addBindValue(faceFile);
    query.addBindValue(dept);

    return query.exec();
}

bool DatabaseManager::insertVisitRecord(QString user_id)
{
    QSqlQuery query;
    int ret = query.prepare("INSERT INTO VisitRecordTable"
                   "(user_id)"
                   "VALUES(?)"
                   );
    if (!ret) {
        qDebug() << query.lastError().text() << endl;
        return false;
    }
    query.addBindValue(user_id);

    return query.exec();
}
