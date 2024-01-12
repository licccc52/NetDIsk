#include "tcpclient.h"
#include "ui_tcpclient.h"
#include<QByteArray>
#include<QDebug>
#include <QHostAddress>
#include<QMessageBox>
#include"protool.h"
#include"privatechat.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(500, 300);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

// 配置文件的加载  加载IP 和 映射的端口号
void TcpClient::loadConfig()
{
    //作为资源文件的打开方式 以冒号开头
    QFile file(":/client.config");
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

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}


//void TcpClient::on_send_pb_clicked()
//{
//    QString strMsg = ui -> lineEdit -> text();
//    //qDebug() << strMsg;
//    if(!strMsg.isEmpty()){
//        PDU *pdu = mkPDU(strMsg.size());
//        pdu -> uiMsgType = 8888;
//        memcpy(pdu -> caMsg, strMsg.toStdString().c_str(), strMsg.size());
//        //memcpy(pdu -> caMsg, &strMsg, strMsg.size());
//        //strMsg.toStdString().c_str(): 这部分首先将一个 Qt 字符串 strMsg 转换为标准 C++ 字符串 (std::string)，
//        //然后使用 c_str() 函数获取该字符串的指针。c_str() 返回一个指向以 null 结尾的 C 字符串的指针，即一个 const char*。
//        //qDebug() << (char*)(pdu->caMsg);
//        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
////        调用 write() 函数将数据放入发送缓冲区，然后由 Qt 的事件循环机制负责将数据发送到远程主机。
////        这意味着在调用 write() 后，数据可能不会立即发送，而是会等待适当的时机发送，这取决于网络状况、系统负载以及其他因素。
////        如果你需要确定数据是否确实已经发送，可以借助信号 bytesWritten(qint64 bytes)。
////        这个信号在 QTcpSocket 成功写入指定数量的字节到套接字时被触发。你可以连接这个信号到一个槽函数，在槽函数中进行进一步的操作或者确认数据已被发送。
//        free(pdu);
//        pdu = NULL;
//    }
//    else{
//        QMessageBox::warning(this, "信息发送", "发送的信息不能为空");
//    }
//}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le -> text();
    m_strLoginName = strName;
    m_strCurPath = strName;
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        PDU* pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32); // 把用户名和密码同时放到了caData里面
        strncpy(pdu->caData+32, strName.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this, "登录", "登录失败:用户名或者密码不能为空");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le -> text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        PDU* pdu = mkPDU(0);
        pdu -> uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32); // 把用户名和密码同时放到了caData里面
        strncpy(pdu->caData+32, strName.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this, "注册", "注册失败:用户名或者密码不能为空");
    }

}

void TcpClient::on_canel_pb_clicked()
{

}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus()){
    qDebug() << "TcpClient::recvMsg()->当前可读数据:"<<m_tcpSocket.bytesAvailable(); // 这个函数获取当前可读数据一共有多少, 可能造成混乱, 例如两个数据一起过来的话就会混乱
    uint uiPDULen = 0;
    m_tcpSocket.read((char*) &uiPDULen, sizeof(uint)); // 先获取总的协议数据单元大小
    uint uiMsgLen = uiPDULen - sizeof(PDU); // sizeof(PDU) 只会计算前面四个元素的大小的和, 不会包括后面那个caMsg[]
    PDU* pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint)); // 先读了前uint大小的数据, 现在要读取之后的
//    sizeof(PDU) 的大小计算方法如下：
//    uiPDULen 和 uiMsgType 分别占用 4 字节（uint 类型）。
//    caData 数组占用了 64 个字符，每个字符占用 1 字节，所以 caData 占用 64 字节。
//    uiMsgLen 占用 4 字节（uint 类型）。
//    caMsg[] 数组没有指定大小，这在 C/C++ 中是一个灵活数组成员（Flexible Array Member）。这种情况下，结构体的大小不包括这个灵活数组成员的大小。
//    所以，sizeof(PDU) 将是 4 + 4 + 64 + 4 = 76 字节。注意，这个大小不包括 caMsg[] 的大小，因为这个数组是一个灵活数组成员，它的大小会根据实际分配的数组长度而变化。
    qDebug()<<"pdu->uiMsgType" << pdu ->uiMsgType;
    switch(pdu->uiMsgType){
    case ENUM_MSG_TYPE_REGIST_RESPOND:
     {
        if(0 == strcmp(pdu->caData, REGIST_OK)){
            QMessageBox::information(this, "注册成功", REGIST_OK);
        }else if(0 == strcmp(pdu->caData, REGIST_FAILED)){
            QMessageBox::warning(this, "注册失败", REGIST_FAILED);
        }
        break;
    }
        //exec()是QApplication类的成员函数,用于进入Qt事件循环并阻塞程序执行,
        //它会启动应用程序的主事件循环,并等待退出信号或者exit()函数,通常在创建了QWidget对象之后,通过调用exec()来显示窗口并开始响应用户的交互
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0 == strcmp(pdu->caData, LOGIN_OK)){
            m_strCurPath = QString("./%1").arg(m_strCurPath);
            QMessageBox::information(this, "登录成功", LOGIN_OK);
            OpeWidget::getInstance().show(); //show()是QWidget类的成员函数, 用于显示窗口或小部件, 它将窗口或者小部件设置为可见状态,并将其添加到父级控件中
            this->hide(); //登录成功之后 把登录界面隐藏
        }else if(0 == strcmp(pdu->caData, LOGIN_FAILED)){ //  -> 如果没有指定父级控件,则它将以独立窗口的形式显示
            QMessageBox::warning(this, "登录失败", LOGIN_FAILED);
        }
        break;

    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        qDebug() << pdu->caData;
        if(0 == strcmp(pdu->caData,SEARCH_USR_NO)){
            QMessageBox::information(this, "搜索", QString("'%1': not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }else if(0 == strcmp(pdu->caData,SEARCH_USR_ONLINE)){
            QMessageBox::information(this, "搜索", QString("'%1': online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(pdu->caData,SEARCH_USR_OFFLINE)){
            QMessageBox::information(this, "搜索", QString("'%1': offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:{

        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32); //前面的是要添加的好友的名字 ->客户端接收到之后, caPerName代表是自己的名字
        strncpy(caName, pdu->caData+32, 32); // 后面的是自己的名字, 代表的是添加好友发起的那个用户
        int ret = QMessageBox::information(this, "添加好友", QString("%1 wang to add you as friend ?").arg(caName), QMessageBox::Yes, QMessageBox::No);
                          //参数 1父窗口, 2title, 3提示消息, 4button(指定按钮)
        PDU* respdu = mkPDU(0);
        memcpy(respdu->caData, pdu->caData, 64);
        if(ret == QMessageBox::Yes){ // 把用户的操作发送给服务器
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        }else{
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:{
        qDebug() << "ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: " << pdu->caData;
        QMessageBox::information(this, "添加好友", pdu->caData);
        break;
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:{
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友",QString("'%1':refused").arg(caName));
        break;
        }
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:{
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友",QString("'%1' has become your friend now!").arg(caName));
        break;
    }
    case ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND:{
        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData,32);
        QMessageBox::information(this, "DELETE FRIEND", QString("%1 delete you from his friend list").arg(caName));
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:{
        QMessageBox::information(this, "DELETE FRIEND", "delete success");
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:{
        qDebug() << " CLIENT ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:" <<pdu->caMsg;
        char caSendName[32] = {'\0'};
        memcpy(caSendName, pdu->caData, 32);
        QString strSendName = caSendName;
        PrivateChat::getInstance().setChatName(strSendName);
        PrivateChat::getInstance().updateMsg(pdu);
        if(PrivateChat::getInstance().isHidden()){

            PrivateChat::getInstance().show(); // 接收到信息之后, 如果想要发送信息的话, 需要提前设置好收信方的名字
        }
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:{
        OpeWidget::getInstance().getFriend()->updateGroupMessage(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REPONSE:{
        QMessageBox::information(this, "创建文件夹", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FRUSH_FILE_REPONSE:{
        OpeWidget::getInstance().getBook()->updateFileList(pdu);
        QString strEnterDir = OpeWidget::getInstance().getBook()->enterDir();
        if(!strEnterDir.isEmpty()){
            m_strCurPath = m_strCurPath+"/"+strEnterDir;
            qDebug() << "ENUM_MSG_TYPE_FRUSH_FILE_REPONSE: " << "!strEnterDir.isEmpty():"<<"enter dirr: " << strEnterDir;
        }
        break;
    }
    case ENUM_MSG_TYPE_DEL_DIR_REPONSE:{
        QMessageBox::information(this, "删除文件夹", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REPONSE:{
        QMessageBox::information(this, "重命名文件", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_REPONSE:{
        OpeWidget::getInstance().getBook()->clearEnterDir();
        QMessageBox::information(this, "进入文件夹" , pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_REPONSE:{
        QMessageBox::information(this, "上传文件", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_REPONSE:{
        QMessageBox::information(this,"删除文件", pdu->caData);
        OpeWidget::getInstance().getBook()->frushFile();
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_REPONSE:{
        qDebug() << "Download" << "ENUM_MSG_TYPE_DOWNLOAD_FILE_REPONSE "<<pdu->caData;
        char caFileName[32] = {'\0'};
        sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
        if(strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0){
            OpeWidget::getInstance().getBook()->setDownloadFlag(true); //设置为下载状态
            m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
            if(!m_file.open(QIODevice::WriteOnly)){ //把文件打开了
                //只写状态下打开文件
                QMessageBox::warning(this, "下载文件", "获得保存文件的路径失败");
            }
        }
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REPONSE:{
        QMessageBox::information(this, "共享文件", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE:{
        qDebug() << "tcpclient.cpp, ENUM_MSG_TYPE_SHARE_FILE_NOTE";
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        qDebug() << "tcpclient: ENUM_MSG_TYPE_SHARE_FILE_NOTE,pdu->caMsg:  " <<pdu->caMsg;
        char *pos = strrchr(pPath, '/'); // 从后面找一个字符, 找到了返回字符的地址
        //strrchar() :从字符串的末尾开始向前搜索，查找指定字符在字符串中最后一次出现的位置。如果找到该字符，则返回从该字符到字符串结束的所有字符的地址；如果没有找到，则返回NULL。
        qDebug() <<"tcpclient: "<< "ENUM_MSG_TYPE_SHARE_FILE_NOTE" << pos;
        if(NULL != pos){
            //找到了
            qDebug() << "ENUM_MSG_TYPE_SHARE_FILE_NOTE, NULL != pos" << pPath;
            pos++;
            QString strNote = QString("%1 share file -> %2\n Do you accept this file?").arg(pdu->caData).arg(pos);
            int ret = QMessageBox::question(this, "共享文件", strNote);
            if(QMessageBox::Yes == ret){
                //愿意接收
                PDU *respdu = mkPDU(pdu->uiMsgLen);
                respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPONSE;
                memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                QString strname = TcpClient::getInstance().loginName(); //接收端的名字
                strcpy(respdu->caData, strname.toStdString().c_str());
                m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            }
        }else{
            qDebug() << "ENUM_MSG_TYPE_SHARE_FILE_NOTE, NULL == pos : " << pPath;
        }
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_REPONSE:{
        QMessageBox::information(this, "移动文件", pdu->caData);
    }
    default:
        break;
    }
    free(pdu);
    pdu=NULL;
    }
    else{ //处于收数据的状态
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if(pBook->m_iTotal == pBook->m_iRecved){
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadFlag(false);
            QMessageBox::information(this, "下载文件", "下载文件成功");
        }else if(pBook->m_iTotal < pBook->m_iRecved){
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadFlag(false);

            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }

}
