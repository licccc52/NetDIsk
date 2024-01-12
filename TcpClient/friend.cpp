#include "friend.h"
#include"protool.h"
#include "tcpclient.h"
#include<QInputDialog>
#include"privatechat.h"
#include"QMessageBox"
Friend::Friend(QWidget *parent) : QWidget(parent)
{
    //关于 Friend 操作的界面
    m_pShowMsgTE = new QTextEdit;   //显示和编辑多行文本.
    m_pFriendListWidget = new QListWidget; // 用于显示列表项的小部件, 通常用于显示项目列表
    m_pInputMsgLE = new QLineEdit;  //单行文本输入框, 用于用户输入单行文本数据.

    m_pDelFriendPB = new QPushButton("删除好友");   //创建按钮
    m_pFlushFriendPB = new QPushButton("刷新好友列表");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户 ");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout* pRightPBVBL = new QVBoxLayout;  // 创建一个垂直管理布局 ,为各种功能按钮创建一个竖直布局
    pRightPBVBL->addWidget(m_pDelFriendPB);      //将一系列按钮添加到'pRightPBVL'的布局中
    pRightPBVBL->addWidget(m_pFlushFriendPB);    //addWidget() 用于将一个控件添加到 "布局" 中
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout* pTopHBL = new QHBoxLayout;     //创建一个水平管理布局
    pTopHBL->addWidget(m_pShowMsgTE);           //显示信息
    pTopHBL->addWidget(m_pFriendListWidget);    //显示朋友列表 //
    pTopHBL->addLayout(pRightPBVBL);    // 显示各种功能按钮 addLayout() 将一个布局管理器添加到另一个布局管理器中作为一个子布局

    QHBoxLayout* pMsgHBL = new QHBoxLayout;
    pMsgHBL -> addWidget(m_pInputMsgLE);
    pMsgHBL -> addWidget(m_pMsgSendPB);


    m_pOnline = new Online;

    QVBoxLayout* pMain = new QVBoxLayout; //垂直管理布局
    pMain->addLayout(pTopHBL);            //消息界面和按钮
    pMain->addLayout(pMsgHBL);            //输入框
    pMain->addWidget(m_pOnline);          //显示在线好友的按钮
    m_pOnline->hide();                    //先隐藏, 点击按钮后开启


    setLayout(pMain);
    connect(m_pShowOnlineUsrPB, SIGNAL(clicked()), this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked()), this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked()), this, SLOT(frushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked()), this, SLOT(deleteFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked()), this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked()), this, SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu) // 显示所有在线用户, UI操作
{
    if(NULL == pdu){
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen/32;
    char caName[32] = {'\0'};
    m_pFriendListWidget->clear();
    for(uint i = 0; i < uiSize; i++){
        memcpy(caName, (char*)(pdu->caMsg)+i*32, 32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updateGroupMessage(PDU *pdu)
{
    QString strMsg = QString("%1 : %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    qDebug() << "updateGroupMessage:" << strMsg;
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()    //查询在线好友 -> 发送到服务器端
{
    if(m_pOnline->isHidden()){
        m_pOnline->show();
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this,"搜索", "用户名");
    if(!m_strSearchName.isEmpty()){
        PDU* pdu = mkPDU(0);
        memcpy(pdu->caData,m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        //toStdString() 返回一个 std::string 类型的对象，然后 c_str() 函数返回一个指向以 null 结尾的字符数组的指针。这个指针作为源地址被传递给 memcpy() 函数。
        //memcpy(pdu->caData,&m_strSearchName, m_strSearchName.size()); 不行
        //&m_strSearchName是一个指向'QString'对象的指针, 不是指向字符串内容的指针, 直接使用这个指针可能会导致复制的不是预期的字符串内容
        qDebug()<<"searchUsr(): " << pdu->caData;
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::frushFriend() // 刷新好友列表 ,返回在线的好友
{
    QString strName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size()); // 先把字符串转换为标准字符串, 然后获得字符串的实际地址, 再获得字符串的大小
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    if(NULL != m_pFriendListWidget->currentItem())
    {
        QString strFriendName = m_pFriendListWidget->currentItem()->text();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        QString strSelfName = TcpClient::getInstance().loginName();
        memcpy(pdu->caData, strSelfName.toStdString().c_str(), strSelfName.size());
        memcpy(pdu->caData+32, strFriendName.toStdString().c_str(), strFriendName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::privateChat()
{
    if(NULL != m_pFriendListWidget->currentItem()){
        QString strChatName = m_pFriendListWidget->currentItem()->text();//选择私聊的对象
        PrivateChat::getInstance().setChatName(strChatName);//此函数调用之后,私聊对象的名字和 我方的名字都被设置好了
        if(PrivateChat::getInstance().isHidden()){
            PrivateChat::getInstance().show();
        }
    }else{
        QMessageBox::warning(this, "私聊", "Please select your chat target");
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if(!strMsg.isEmpty()){
        PDU *pdu = mkPDU(strMsg.size()+1);
        pdu -> uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName = TcpClient::getInstance().loginName();
        strncpy(pdu->caData, strName.toStdString().c_str(), strName.size()); // string先转换为标准字符串, 然后再获取地址
        strncpy((char*)(pdu->caMsg), strMsg.toStdString().c_str(), strMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        updateGroupMessage(pdu);
        free(pdu);
        pdu = NULL;
    }else{
        QMessageBox::warning(this, "群聊", "发送的消息不能为空");
    }
}
