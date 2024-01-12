#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QStringList>

class OpeDB : public QObject
{
    Q_OBJECT // 包括Q_OBJECT, 使这个类支持信号槽
public:
    explicit OpeDB(QObject *parent = nullptr);
    //单例对象 , 定义一个静态的成员函数,返回一个静态的对象
    static OpeDB& getInstance();
    //static OpeDB instance;
    //return instance;
    void init();
    ~OpeDB();

    bool handleRegist(const char* name, const char* pwd);
    bool handleLogin(const char* name, const char* pwd);
    void handleOffline(const char* name);
    QStringList handAllOnline();
    int handleSearchUsr(const char* name); //查找的用户有三种状态, 1. 用户存在并在线 2.用户存在但不在线 3.用户不存在
    int handleAddFriend(const char* pername, const char* name);
    void addFriend(const char *pername, const char *name);
    QStringList handleFrushFriend(const char *name); // 根据名字找到对应的好友
    bool handleDeleteFriend(const char* name, const char *friendName);

private:
    QSqlDatabase m_db; // 连接数据库

signals:

};

#endif // OPEDB_H
