#include "mytcpserver.h"
#include<QDebug>

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
   QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
   for(;iter != m_tcpSocketList.end(); iter++){
       if(mysocket == *iter){
           delete *iter;  //删除指针指向的对象
           *iter = NULL;  //把指针指向空
           m_tcpSocketList.erase(iter); // 把列表中的指针删除, 通过迭代器
           break;
       }
   }
   //删除目标Socket之后再遍历输出所有Socket
   for(int i= 0; i < m_tcpSocketList.size();i++){
       qDebug() << m_tcpSocketList.at(i)->getName();
   }
}

MyTcpServer::MyTcpServer()
{

}

//到后面凡是需要用到MyTcpServer的地方, 都可以用这个调用这个函数 得到一个MyTcpServer的引用 静态的局部对象
MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance; // 静态的局部对象, 无论调用多少次, 这个对象有且只有一个
    return instance;
}
//在 Qt 的 QTcpServer 类中，默认的行为是当有新连接到达时，会自动调用 incomingConnection() 函数。
void MyTcpServer::incomingConnection(qintptr  handle) //qintptr  handle是一个指向新连接的句柄的整数值,
{
    qDebug() << "Connection coming";
    MyTcpSocket* pTcpSocket = new MyTcpSocket;  // 建立一个新socket来处理这个连接
    pTcpSocket->setSocketDescriptor(handle); // setSocketDescriptor() 用于将一个现有的本地套接字描述符和Qt套接字绑定在一起,使用setSocketDescriptor函数可以
                                             //将现有的套接字与Qt套接字一同管理, 方便数据收发
    m_tcpSocketList.append(pTcpSocket); // 关联客户端连接 和 数据收发的 socket
    connect(pTcpSocket, SIGNAL(offLine(MyTcpSocket*)), this, SLOT(deleteSocket(MyTcoSocket*))); // 收到Socket的下线信号之后, 在tcpsocket列表中把该socket删除
}

void MyTcpServer::resend(const char* pername, PDU *pdu)
{

    if(NULL == pername || NULL == pdu){
        return;
    }
    QString strName = pername;
    for(int i= 0; i < m_tcpSocketList.size(); i++){
        if(strName == m_tcpSocketList.at(i)->getName()){
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);//使用对应的套接字, 发送给对应的用户
            qDebug() << "resend():  m_tcpSocketList.at(i)->getName() " << m_tcpSocketList.at(i)->getName() << "pdu->msgType" << pdu->uiMsgType;
            break;
        }
    }

}
