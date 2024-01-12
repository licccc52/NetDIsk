#include "book.h"
#include"tcpclient.h"
#include<QInputDialog>
#include<QMessageBox>
#include<QDebug>
#include<QFileDialog>
#include"opewidget.h"
#include"sharefile.h"
Book::Book(QWidget *parent) : QWidget(parent)
{
    m_strEnterDir.clear();

    m_pTimer = new QTimer;
    m_bDownload = false;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件");
    m_pFrushPB = new QPushButton("刷新文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFrushPB);

    m_puploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("分享文件");

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_puploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);
    connect(m_pCreateDirPB, SIGNAL(clicked()), this, SLOT(createDir()));
    connect(m_pFrushPB, SIGNAL(clicked()), this, SLOT(frushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked()), this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked()), this, SLOT(renameFile()));

//    connect(m_pBookListW, SIGNAL(doubleClicked()), this, SLOT(enterDir()));
    connect(m_pBookListW, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(enterDir(QListWidgetItem*)));
    connect(m_pReturnPB, SIGNAL(clicked()), this, SLOT(returnPre()));
    connect(m_puploadPB, SIGNAL(clicked()), this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked()), this, SLOT(delRegFile()));
    connect(m_pDownloadPB, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked()), this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked()), this, SLOT(moveFile()));
    connect(m_pSelectDirPB, SIGNAL(clicked()), this, SLOT(selectDestDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    m_pBookListW->clear();
//    QListWidgetItem *pItemTmp = NULL;
//    int row = m_pBookListW->count();
//    while(m_pBookListW->count() > 0){
//        pItemTmp = m_pBookListW->item(row-1);
//        m_pBookListW->removeItemWidget(pItemTmp);
//        delete pItemTmp;
//        row = row - 1;
//    }
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i = 0; i < iCount; i++){
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        QListWidgetItem* pItem = new QListWidgetItem;

        if(0 == pFileInfo->iFileType){
            qDebug() << pFileInfo->iFileType;
            pItem->setIcon(QIcon(QPixmap(":/folder.jpg")));}
        else if(1 == pFileInfo->iFileType)
        {pItem->setIcon(QIcon(QPixmap(":/file.jpg")));
        }

        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}



void Book::createDir()  //功能:在当前目录下面创建一个心得文件夹; 参数: 用户名, 要创建的文件夹的名字, 当前所在的路径信息
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "请输入新文件夹的名字"); // title 和 提示语
    if(!strNewDir.isEmpty()){
        //非空的情况下
        QString strName = TcpClient::getInstance().loginName();
        QString strCurPath = TcpClient::getInstance().curPath();
        qDebug() <<"strCurPath" << strCurPath;
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
        strncpy(pdu->caData+32, strNewDir.toStdString().c_str(), strNewDir.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        QMessageBox::warning(this,"新建文件夹", "新文件夹名字不能为空");
    }
}

void Book::frushFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_REQUEST;
    strncpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()//功能: 删除用户指定的文件
{//发送了两个信息, 当前路径 和 要删除的文件的名字
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获取当前的选项
    if(NULL == pItem){
        QMessageBox::warning(this, "删除文件", "请选择你要删除的文件");
    }else{ //所选文件 非空
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1); // 封装pdu, 准备发送
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameFile()
{
    QString curPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获取当前的选项
    if(NULL == pItem){
        QMessageBox::warning(this, "重命名文件", "请选择你要重命名的文件");
    }else{//选的文件非空, 获得文件的名字,然后发送给服务器
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "重命名文件","请输入新的文件名");
        if(!strNewName.isEmpty()){
            PDU *pdu = mkPDU(curPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData+32, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMsg, curPath.toStdString().c_str(), curPath.size());
            qDebug() << "重命名文件: " << (char*)pdu->caMsg;
            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }else{//
            QMessageBox::warning(this, "重命名文件", "新文件的名字不能为空");
        }
    }
}

void Book::enterDir(QListWidgetItem *item) // 和双击关联在一起的槽函数, 进入某个文件夹
{
    QString strDirName = item->text();
    m_strEnterDir = strDirName; // 记录进入的文件夹, 用m_strEnterDir保存
    qDebug() << "entryDir()" << strDirName;
    QString curPath = TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size()); //选择的文件
    memcpy(pdu->caMsg, curPath.toStdString().c_str(), curPath.size()); //现在的路径
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::enterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadFlag(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    if(strCurPath == strRootPath){
        QMessageBox::warning(this, "返回", "返回失败: 已经在最开始的文件夹目录中了");
    }
    else{
        // "./aa/bb/cc" --> "./aa/bb"
        int index = strCurPath.lastIndexOf('/'); //找字符串中中最后一个 '/' 出现的位置, 返回一个整数
        strCurPath.remove(index, strCurPath.size()-index);//删除 '/' 后面的所有内容
        qDebug() <<"Book-> " << "returnPre: " << strCurPath;
        TcpClient::getInstance().setCurPath(strCurPath);
        clearEnterDir();
        frushFile();
    }
}

void Book::uploadFile()
{
    QString curPath = TcpClient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName(); //弹出一个窗口, 选择文件, 返回选择一个文件的路径
    qDebug() <<"Book: " << "strUploadFilePath--" << m_strUploadFilePath;//返回的文件路径是选择的文件的绝对路径
    //pdu中包含的信息是文件的路径和文件名字
    if(!m_strUploadFilePath.isEmpty()){
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        qDebug() <<"uploadFile: " <<"strfileName " << strFileName;

        QFile file(m_strUploadFilePath); // 通过file对象获得文件的大小
        qint64 fileSize = file.size();//获得文件的大小
        PDU *pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, curPath.toStdString().c_str(), curPath.size()); // 当前路径
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);//文件名和文件大小
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
        //上传文件的请求刚发送出去的话, 不能直接发送文件, 会出现粘包的情况, 文件的内容和刚发送的请求会粘在
        m_pTimer -> start(1000);// 1s之后上传数据开始
    }else{
        QMessageBox::warning(this, "上传文件", "上传文件名字不能为空");
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }

    char *pBuffer = new char[4096]; //每次读写4096大小的数据, 综合读写效率比较高
    qint64 ret = 0;
    while(true){
        ret = file.read(pBuffer, 4096); //ret表示实际读到的数据
        if(ret > 0 && ret <= 4096){
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }else if(0 == ret){
            break;
        }else{
            //ret < 0 -> 出错
            QMessageBox::warning(this, "上传文件", "上传文件失败: 读取文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem(); // 获取当前的选项
    if(NULL == pItem){
        QMessageBox::warning(this, "删除文件", "请选择你要删除的文件");
    }else{ //所选文件 非空
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1); // 封装pdu, 准备发送
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    //
    QString strSaveFilePath = QFileDialog::getSaveFileName();//弹出一个窗口, 用户可以自己选择一个路径
    if(strSaveFilePath.isEmpty()){
        QMessageBox::warning(this, "下载文件", "请指定要保存的位置");
        m_strSaveFilePath.clear();
    }else{
        m_strSaveFilePath = strSaveFilePath;
    }
    //这一段需要放在前面, 如果放在后面的话, socket发送信息之后, 服务器端收到消息, 会直接发送回来,并且检查
    //保存文件的路径是否选择好了, 这段事件远远小于人操作的时间, 所以需要让这一段代码执行完毕之后,
    //再调用socket向服务器端发送数据
   QListWidgetItem *pItem = m_pBookListW->currentItem();
   if(NULL == pItem){
       QMessageBox::warning(this, "下载文件", "请你选择要下载的文件");
   }else{
       QString strCurPath = TcpClient::getInstance().curPath();
       PDU *pdu = mkPDU(strCurPath.size()+1);
       pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
       QString strFileName = pItem->text();
       strcpy(pdu->caData, strFileName.toStdString().c_str());
       memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
       TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
   }
}

void Book::shareFile()
{
 //点击分享按钮之后, 向自己的好友们分享文件
    //获得当前分享的文件
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem){
        QMessageBox::warning(this, "分享文件", "请选择要分享的文件");
        return;
    }else{
        m_strShareFileName = pItem->text();
    }
    //通过包含OpeWwidget.h, 获得opewidget对象, 获得其中的成员friend, 得到好友列表
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    qDebug() << "shareFile: " << " pFriendList";
    ShareFile::getInstance().upadteFriend(pFriendList);
    if(ShareFile::getInstance().isHidden()){
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL != pCurItem){
        //用户选择文件之后
        m_strMoveFileName = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }else{
        //没有选择文件
        QMessageBox::warning(this, "移动文件", "请选择要移动的文件");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL != pCurItem){
        //用户选择文件之后
        QString strDestDir = pCurItem->text();
        QString strCurPath = TcpClient::getInstance().curPath();
        m_strDestDir = strCurPath + '/' + strDestDir;

        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strDestDir.size();
        PDU *pdu = mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData, "%d %d %s",srcLen, destLen, m_strMoveFileName.toStdString().c_str());
        memcpy((char*)pdu->caMsg, m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char*)(pdu->caMsg)+(srcLen+1), m_strDestDir.toStdString().c_str(), destLen);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        //没有选择文件
        QMessageBox::warning(this, "移动文件", "请选择目标路径");
    }
    m_pSelectDirPB->setEnabled(false);
}




