#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include<QTcpSocket>
#include<QDir>
#include<QFile>
#include<QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT; // 支持信号槽
public:
    MyTcpSocket();
    QString getName();
    void copyDir(QString strSrcDir, QString strDestDir);

signals:
    void offLine(MyTcpSocket *mysocket);

public slots:
    void recvMsg();
    void test();
    void clientOffline(); // 用来处理客户端用户下线的 槽函数
    void senFileToClient();

private:
    QString m_strName;
    QFile m_file;
    qint64 m_iTotal; //记录文件总的大小
    qint64 m_iRecved; //记录文件已经接收了多少, 如果已经大于文件的尺寸则结束
    bool m_bUpload; //记录 显示 是否处于文件上传中的状态
    QTimer *m_pTimer;

};

#endif // MYTCPSOCKET_H
//newConnection() 是一个信号，当 QTcpServer 接收到新的连接时被发射。
//连接建立时，会触发这个信号，告诉你有新的连接已经建立。你可以将这个信号与特定的槽函数相连，以便在新连接建立时执行特定的操作。
