#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include<QTcpServer>
#include<QList>  //定义一个列表 , 把所有的socket保存到列表中
#include"mytcpsocket.h"
#include"protool.h"

class MyTcpServer : public QTcpServer// QTcpServer 继承了 QOBJECT
{
    //为了让这个类支持信号槽
    //添加Q_OBJECT宏 使 这个类支持信号槽 机制
    Q_OBJECT;
public slots:
    void deleteSocket(MyTcpSocket* mysocket);
public:
    MyTcpServer();

    //单例模式
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr handle);
    void resend(const char* pername, PDU* pdu );


private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
