#include "opedb.h"
#include<QMessageBox>
#include<QDebug>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;

}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("C:\\Qt_projects\\TcpServer\\cloud.db");
    if(m_db.open()){
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next()){
            QString data = QString("%1,%2,%3").arg(query. value(0).toString()).arg(query. value(1).toString()).arg(query. value(2).toString());
            qDebug() << data;
        }
    }
    else{
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(NULL == name || NULL == pwd){
        return false; //考虑形参的有效性
    }
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\', \'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL == name || NULL == pwd){
        return false; //考虑形参的有效性
    }
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data); //使用之前构建好的SQL查询语句来执行数据库查询,把这个查询发送到数据库并执行,返回执行结果
    if(query.next()){ // 在查询到数据库中有 这个用户的数据之后, 需要改变其登录状态, 把登录状态设置为1, 防止重复登录
        QString data = QString("update usrInfo set online=1 where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
        qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        return true;
    }
    else {
        return false;
    }
}

void OpeDB::handleOffline(const char *name) // 操作数据库, 把改变下线状态
{
    if(NULL == name){
         qDebug() << "name is null";
         return;
    }
    QString data = QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handAllOnline()
{
    QString data = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next()){
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(NULL == name){
        return -1;
    }
    qDebug() << "handleSearchUsr: " << name;
    QString data = QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    qDebug() << data;
    if(query.next()){
        int ret = query.value(0).toInt();
        if(1 == ret){
            return 1;
        }else if(0 == ret){
            return 0;        }
    }else{
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name) //返回值 -1(操作无效), 0(已经是好友了), 1(不是好友,在线), 2(不是好友不在线), 3(不存在这个用户)
{
    if(NULL == pername || NULL == name){
        return -1; // 系统故障, 添加好友失败, || 您的操作无效
    }
    // 数据库friend表中 有 id 和 friendID两个字段
    QString data = QString("select * from friend where id = (select id from UsrInfo where name = \'%1\') and friendID = (select id from UsrInfo where name = \'%2\') "
                           "or friendId = (select id from UsrInfo where name = \'%3\') and id = (select id from UsrInfo where name = \'%4\'))").arg(pername).arg(name).arg(name).arg(pername);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        return 0; //表示双方已经是好友了
    }
    else{
        //不是好友了 , 需要查询对方是否在线
        QString data = QString("select online from usrInfo where name=\'%1\'").arg(pername); // pername代表对方的名字
        QSqlQuery query;
        query.exec(data);
        if(query.next()){
            int ret = query.value(0).toInt();
            if(1 == ret){ // 对方用户在线
                return 1;
            }else if(0 == ret){ // 对方用户不在线
                return 2;        }
        }else{
            return 3; // 对方不存在这个人
        }
    }
}

void OpeDB::addFriend(const char *pername, const char *name)
{
    QString data = QString("insert into friend (id, friendId) values((select id from Usrinfo where name = \'%1\')"
                           ",(select id from Usrinfo where name = \'%2\'))").arg(name).arg(pername);
    QSqlQuery query;
    query.exec(data);
    qDebug() << data;
    return;
}

QStringList OpeDB::handleFrushFriend(const char *name) // 获得了所有在线好友的名字List
{
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name){
        return strFriendList;
    }
    QString data = QString("select name from usrInfo where online=1 and id in (select id from friend where friendId = (select id from usrInfo where name = \'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while(query.next()){
        strFriendList.append(query.value(0).toString());
    }

    data = QString("select name from usrInfo where online=1 and id in (select friendId from friend where id = (select id from usrInfo where name = \'%1\'))").arg(name);
    query.exec(data);
    while(query.next()){
        strFriendList.append(query.value(0).toString());
    }

    return strFriendList;
}

bool OpeDB::handleDeleteFriend(const char *name, const char *friendName)
{
    if(NULL == name || NULL == friendName){
        return false;
    }
    QString data = QString("delete from friend where id = (select id from usrInfo where name = \'%1\') "
                           "and friendId = (select id from usrInfo where name = \'%2\')").arg(name).arg(friendName);
    QSqlQuery query;
    query.exec(data);
    data = QString("delete from friend where id = (select id from usrInfo where name = \'%1\') "
                                         "and friendId = (select id from usrInfo where name = \'%2\')").arg(friendName).arg(name);
    query.exec(data);

}
