#ifndef DATABASEMANNAGER_H
#define DATABASEMANNAGER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

struct UserInfo {
    QString name;
    QString workId;
    QString identity;
    QString imgPath;
    QString dept;
    bool valid = false;   // 用来表示是否查到
};
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    static DatabaseManager *getInstance();
    bool initDataBase();
    bool insertPersonInfo(QString user_id,
                          QString identity,
                          QString workId,
                          QString name,
                          QString faceFile,
                          QString dept);
    bool insertVisitRecord(QString user_id);


    UserInfo getInfoByUID(const QString& faceToken);


private:
    static DatabaseManager *self;
    QSqlDatabase db;

};

#endif // DATABASEMANNAGER_H
