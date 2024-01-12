#include "privatechat.h"
#include "ui_privatechat.h"
#include"protool.h"
#include"tcpclient.h" // 使用登录时候保存的用户的名字, 获得我方的名字
#include"QMessageBox"
//Qt设计师界面类, 自带一个UI文件
PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::setChatName(QString strName) //在私聊的时候, 就用这个函数设置聊天对象的名字,
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getInstance().loginName();
}

PrivateChat &PrivateChat::getInstance() //私聊的时候,用这个函数操作私聊界面的窗口
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu){
        return; //有效性判断
    }
    char caSendName[32] = {'\0'};
    memcpy(caSendName, pdu->caData, 32);
    QString strMsg = QString("%1 : %2").arg(caSendName).arg((char*)pdu->caMsg);
    ui->showMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if(!strMsg.isEmpty()){ //输入的信息不为空
        ui->showMsg_te->append(QString("You: %1").arg(strMsg));
        PDU *pdu = mkPDU(strMsg.size()+1); // +1之后要在字符串后面添加一个 '\0'
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.size());
        memcpy(pdu->caData+32, m_strChatName.toStdString().c_str(), m_strChatName.size());
        strcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{//输入的信息为空
        QMessageBox::warning(this, "私聊", "message can't be null");
    }
}
