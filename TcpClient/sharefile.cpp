#include "sharefile.h"
#include"tcpclient.h"
#include"opewidget.h"

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectALlPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false); //单选设置为false;

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectALlPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    connect(m_pCancelSelectPB, SIGNAL(clicked()), this, SLOT(cancelSelect()));
    connect(m_pSelectALlPB, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(m_pOKPB, SIGNAL(clicked()), this, SLOT(okShare()));
    connect(m_pCancelPB, SIGNAL(clicked()), this, SLOT(cancelShare()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::upadteFriend(QListWidget *pFriendList)
{
    if(NULL == pFriendList){
        return;
    }
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    for(int i  = 0; i<preFriendList.size();i++){
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp); // 现在布局中移除friend
        m_pButtonGroup->removeButton(tmp); //再在buttongroup中移除 friend
        preFriendList.removeOne(tmp); //再从List中删除
        delete tmp; //删除指针
    }

    QCheckBox *pcb = NULL;
    for(int i = 0; i<pFriendList->count();i++){
        pcb = new QCheckBox(pFriendList->item(i)->text());
        qDebug() <<"sharefile.cpp: " << "pFriendList: ";
        m_pFriendWVBL->addWidget(pcb);
        m_pButtonGroup->addButton(pcb);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList =  m_pButtonGroup->buttons();
    for(int i=0; i < cbList.size(); i++){
        if(cbList[i]->isChecked()){
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList =  m_pButtonGroup->buttons();
    for(int i=0; i < cbList.size(); i++){
        if(!cbList[i]->isChecked()){
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    // 分享者, 接收者, 当前路径, 文件名
    //PDU->caData: 分享者,分享人数, PDU->caMsg:接收者, 当前路径和文件名拼接成路径放在 PDU->caMsg 中
    QString strSharePer = TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();

    QString strPath = strCurPath + "/" + strShareFileName;

    QList<QAbstractButton*> cbList =  m_pButtonGroup->buttons();
    int num = 0;
    for(int i=0; i < cbList.size(); i++){
        if(cbList[i]->isChecked()){
            num++;
        }
    }

    PDU *pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strSharePer.toStdString().c_str(), num); //pdu->caData 分享人的名字, 接收者的个数
    int j = 0;
    for(int i = 0; i < cbList.size(); i++){
        if(cbList[i]->isChecked()){
            memcpy((char*)(pdu->caMsg)+j*32, cbList[i]->text().toStdString().c_str(), cbList[i]->text().size()); //接收者们的名字
            j++;
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32, strPath.toStdString().c_str(), strPath.size()); // 要分享的文件的路径

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void ShareFile::cancelShare()
{
    hide();
}
