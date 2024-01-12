#include "tcpserver.h"
#include "ui_tcpserver.h"
#include"mytcpserver.h"

#include <QFile>
#include <QMessageBox>
#include<QDebug>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    //服务器进行监听, 需要指定 IP和端口
    //IP和端口已经作为资源文件添加到 项目中了, 所以可以直接复制TcpClient的代码
    TcpServer::loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    //作为资源文件的打开方式 以冒号开头
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
//        qDebug() << strData;
        file.close();
        strData.replace("\r\n", " ");
        qDebug() << strData;
        QStringList strList = strData.split(" ");
//        for(int i = 0; i < strList.size();i++){
//            qDebug() << " -- > " <<strList[i];
//        }

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << m_strIP << " " << m_usPort;
    }
    else{
        QMessageBox::critical(this,"open config", "open config failed");
    }
}

