#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include<QTcpSocket>
#include"protool.h"
#include"opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient& getInstance(); // 使用单例模式, 因为在查询在线好友功能的时候, 需要用tcpsocket对象,
    //TcpClient类中有一个QTcpSocket m_tcpSocket成员变量, 需要在friend.cpp中使用这个类对象, 所有Main函数中的TcpClient对象.show()也需要更改
    QTcpSocket &getTcpSocket();
    QString loginName();
    QString curPath();
    void setCurPath(QString strCurPath);


public slots:
    //添加槽函数 捕获连接成功的信息 : connected()
    void showConnect();

private slots:
//    void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_canel_pb_clicked();

    void recvMsg();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;

    //连接服务器, 和服务器进行数据交互
    QTcpSocket m_tcpSocket;
    QString m_strLoginName; // 存储正在登录的用户的名字

    QString m_strCurPath; //记录当前路径
    QFile m_file;
};
#endif // TCPCLIENT_H
