#include "mytcpsocket.h"
#include "opedb.h"
#include "protool.h"
#include"mytcpserver.h"
#include<QDir>
#include<QFileInfoList>

MyTcpSocket::MyTcpSocket()
{
    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    //readyRead() 是 Qt 中的一个信号，通常与 QTcpSocket 类相关联。当一个 QTcpSocket 实例接收到新数据并且准备好被读取时，就会发出 readyRead() 信号。
    //这个信号表明套接字已经接收到了新的数据，可以通过 read() 或 readAll() 等函数来读取这些数据。
    //通常情况下，你可以连接这个信号到一个槽函数，以便在数据可用时执行特定的操作，比如处理接收到的数据或者进行进一步的处理。
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(senFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir); //创建一个目录, 然后拷贝文件

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString desTmp; //目标路径

    for(int i = 0; i<fileInfoList.size(); i++){
        if(fileInfoList[i].isFile())//如果是常规的文件, 就直接拷贝
        {
            qDebug() << "MyTcpSocket:: copyDir(). ->fileInfoList[i].fileName()"<<fileInfoList[i].fileName();
            fileInfoList[i].fileName();
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            desTmp = strDestDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp, desTmp);
        }
        else if(fileInfoList[i].isDir()){
            if(QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName() ){continue;}
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            desTmp = strDestDir + '/' + fileInfoList[i].fileName();
            copyDir(srcTmp, desTmp);
        }
        qDebug() << "srcTmp:" <<srcTmp << " desTmp: " <<desTmp;
    }
}


void MyTcpSocket::recvMsg()
{
    //判断传输过来的东西是文件还是PDU
    if(!m_bUpload){ //如果不是上传文件的状态, 就用PDU接受
    qDebug() <<"当前可读数据一共有多少: "<< this->bytesAvailable(); // 这个函数获取当前可读数据一共有多少, 可能造成混乱, 例如两个数据一起过来的话就会混乱
    uint uiPDULen = 0;
    this -> read((char*) &uiPDULen, sizeof(uint)); // 先获取总的协议数据单元大小
    uint uiMsgLen = uiPDULen - sizeof(PDU); // sizeof(PDU) 只会计算前面四个元素的大小的和, 不会包括后面那个caMsg[]
    PDU* pdu = mkPDU(uiMsgLen);
    this -> read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint)); // 先读了前uint大小的数据, 现在要读取之后的
//    sizeof(PDU) 的大小计算方法如下：
//    uiPDULen 和 uiMsgType 分别占用 4 字节（uint 类型）。
//    caData 数组占用了 64 个字符，每个字符占用 1 字节，所以 caData 占用 64 字节。
//    uiMsgLen 占用 4 字节（uint 类型）。
//    caMsg[] 数组没有指定大小，这在 C/C++ 中是一个灵活数组成员（Flexible Array Member）。这种情况下，结构体的大小不包括这个灵活数组成员的大小。
//    所以，sizeof(PDU) 将是 4 + 4 + 64 + 4 = 76 字节。注意，这个大小不包括 caMsg[] 的大小，因为这个数组是一个灵活数组成员，它的大小会根据实际分配的数组长度而变化。
    qDebug() << "recvMsg()" << "pdu->uiMsgtype: " << pdu->uiMsgType;
    switch(pdu -> uiMsgType){
    case ENUM_MSG_TYPE_REGIST_REQUEST:
     {  char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);

        bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if(ret){
            strcpy(respdu->caData, REGIST_OK); //登录成功, 创建专属的文件夹
            QDir dir;
            qDebug() <<dir.mkdir(QString("./%1").arg(caName));
        }else{
            strcpy(respdu->caData, REGIST_FAILED);
        }

        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:{
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
        bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret){
            strcpy(respdu->caData, LOGIN_OK);
            m_strName = caName;
        }else{
            strcpy(respdu->caData, LOGIN_FAILED);
        }

        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        //返回了一个字符串列表, 记录了所有在线的用户的名字
        QStringList ret = OpeDB::getInstance().handAllOnline();
        //需要定义一个PDU来回复客户端所有在线的客户的名单
        uint uiMsgLen = ret.size()*32;
        PDU* respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i=0; i < ret.size(); ++i){
             memcpy((char*)(respdu->caMsg)+i*32, ret.at(i).toStdString().c_str(), ret.at(i).size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:{
        qDebug() << "mytcpsocket:"<<pdu->caData;
        int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(-1 == ret){
            strcpy(respdu->caData,SEARCH_USR_NO);
        }else if(1 == ret){
            strcpy(respdu->caData, SEARCH_USR_ONLINE);
        }else if(0 == ret){
            strcpy(respdu->caData, SEARCH_USR_OFFLINE);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:{  // 返回了两种消息类型, -1, 0, 2, 3是respond,新建的pdu, 1的时候是request,使用的还是原来的pdu,所以没有变化
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32); //前面的是要添加的好友的名字
        strncpy(caName, pdu->caData+32, 32); // 后面的是自己的名字
        int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);// 要添加的好友的名字, 自己的名字
        PDU* respdu = NULL;
        if(-1 == ret){ //返回值 -1(操作无效),
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, UNKNOW_ERROR);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }else if(0 == ret){ //0(已经是好友了)
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, EXISTED_FRIEND);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }else if(1 == ret){ //1 不是好友,在线 -> 转发给对方
            MyTcpServer::getInstance().resend(caPerName, pdu); //发送给了对方(被添加的用户)

        }else if(2 == ret){ //2(不是好友不在线), 就不做其他的事情了
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }else if(3 == ret){ // 3 不存在这个用户)
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, ADD_FRIEND_NOEXIST);
            write((char*)respdu, respdu->uiPDULen);
            qDebug() << "该用户不存在";
            free(respdu);
            respdu=NULL;
        }

        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:{
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32); //前面的是要添加的好友的名字
        strncpy(caName, pdu->caData+32, 32); // 后面的是自己的名字
        OpeDB::getInstance().addFriend(caPerName, caName);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        strcpy(respdu->caData, caName);
        MyTcpServer::getInstance().resend(caPerName, respdu);
        strcpy(respdu->caData, caPerName);
        MyTcpServer::getInstance().resend(caName, respdu);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:{ // 服务器端向添加发起人发送 添加好友操作 对方同意与否
        PDU* respdu = mkPDU(0);
        char caName[32] = {'\0'};
        strcpy(caName, pdu->caData+32);
        memcpy(respdu->caData, pdu->caData, 64);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        MyTcpServer::getInstance().resend(caName, respdu); // 把pdu发送到指定的人那里去
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST:{
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData,32);
        QStringList ret = OpeDB::getInstance().handleFrushFriend(caName);
        uint uiMsgLen = ret.size()*32; //根据字符串列表的size来计算有多少名字, 一个人的名字是32个字节
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND;
        for(int i = 0; i < ret.size(); i++){
            memcpy((char*)(respdu->caMsg)+i*32, ret.at(i).toStdString().c_str(), ret.at(i).size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:{
        char caSelfName[32] = {'\0'};
        char caFriendName[32] = {'\0'};
        strncpy(caSelfName, pdu->caData, 32);
        strncpy(caFriendName, pdu->caData+32, 32);
        OpeDB::getInstance().handleDeleteFriend(caSelfName, caFriendName);
        PDU *respdu = mkPDU(0);
        respdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        strcpy(respdu->caData, DEL_FRIEND_OK);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        MyTcpServer::getInstance().resend(caFriendName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:{
        char caPerName[32] = {'\0'};
        memcpy(caPerName, pdu->caData+32, 32);
        MyTcpServer::getInstance().resend(caPerName, pdu);
        qDebug() << "SERVER: ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST" <<pdu->caData;
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: // 群聊:给所有在线的好友发送消息
    {
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        QStringList onlineFriend = OpeDB::getInstance().handleFrushFriend(caName);
        QString tmp;
        for(int i = 0; i < onlineFriend.size(); i++){
            tmp = onlineFriend.at(i);// at(i). 获得名字的首地址
            MyTcpServer::getInstance().resend(tmp.toStdString().c_str(), pdu);
        }
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:{
        QDir dir;
        QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
        qDebug() << "strCurPath:" <<strCurPath;
        bool ret = dir.exists(strCurPath);
        PDU *respdu;
        if(ret){ // 当前目录存在
            char caNewDir[32] = {'\0'};//验证当前目录下有没有和要创建的文件夹 同名的存在
            memcpy(caNewDir, pdu->caData+32, 32);
            QString strNewPath = strCurPath + "/" + caNewDir;
            qDebug()<<"strNewPath:"  << strNewPath;
            qDebug() << "ENUM_MSG_TYPE_CREATE_DIR_REQUEST" << strNewPath;
            bool ret2 = dir.exists(strNewPath);
            if(ret2){//要创建的文件名已经存在
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REPONSE;
                strcpy(respdu->caData, FILE_NAME_EXIST);
            }else{ // 要创建的文件名不存在
                dir.mkdir(strNewPath);
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REPONSE;
                strcpy(respdu->caData, CREATE_DIR_OK);
            }
        }else{ // 当前目录不存在
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REPONSE;
            strcpy(respdu->caData, DIR_NO_EXIST);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FRUSH_FILE_REQUEST:{
        char *pCurPath = new char[pdu->uiMsgLen];
        memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
        QDir dir(pCurPath);
        QFileInfoList fileInfoList =  dir.entryInfoList(); // 以字符串列表的形式 返回 当前目录下的信息
        int iFileCount = fileInfoList.size();
        PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
        respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_REPONSE;
        FileInfo *pFileInfo = NULL;
        QString strFileName;
        for(int i = 0; i < iFileCount; i++){
            pFileInfo = (FileInfo*)respdu->caMsg+i; // 指针+1, 代表地址上加上地址类型大小的数量, 将respdu->caMsg后面的地址转换为FileInfo*类型, 然后用一个地址变量指向他, 对这块地址进行操作
            strFileName = fileInfoList[i].fileName();
            memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size()); //向内存中拷贝名字, 之后需要判断文件类型
            if(fileInfoList[i].isDir()){ //判断文件类型, 是文件夹 还是文件, 或者是其他的什么类型的文件
                pFileInfo->iFileType = 0;
            }else if(fileInfoList[i].isFile()){
                pFileInfo->iFileType = 1;
            }
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        pdu = NULL;
        break;
        }
    case ENUM_MSG_TYPE_DEL_DIR_REQUEST:{
        char caName[32] = {'\0'}; // 拷贝名字
        strcpy(caName, pdu->caData); // caData为char类型
        char *pPath = new char[pdu->uiMsgLen];//拷贝路径
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);//拼接
        QFileInfo fileInfo(strPath);
        bool ret = false;
        if(fileInfo.isDir()){//判断是否是一个文件的路径
            QDir dir;
            dir.setPath(strPath);
            ret = dir.removeRecursively(); // Removes the directory, including all its contents
                                    //删除路径, 包括路径中所有的内容(文件)
        }else if(fileInfo.isFile()){//是一个常规文件}
            ret = false; // 不删除常规文件, 只删除文件夹
        }
        PDU *respdu = NULL;
        if(ret){ //是一个文件夹, 可以删除, 删除成功
            respdu = mkPDU(strlen(DEL_DIR_OK)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REPONSE;
            memcpy(respdu->caData, DEL_DIR_OK, strlen(DEL_DIR_OK));
        }else{ // 是一个常规文件, 返回删除失败
            respdu = mkPDU(strlen(DEL_DIR_FAILURED)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REPONSE;
            memcpy(respdu->caData, DEL_DIR_FAILURED, strlen(DEL_DIR_FAILURED));
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:{
        char caOldName[32] = {'\0'};
        char caNewName[32]= {'\0'};
        char *curPath = new char[pdu->uiMsgLen];//拷贝当前路径
        strncpy(caOldName, pdu->caData,32);
        strncpy(caNewName, pdu->caData+32, 32);
        memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
        QString strOldPath = QString("%1/%2").arg(curPath).arg(caOldName);
        QString strNewPath = QString("%1/%2").arg(curPath).arg(caNewName);
        qDebug() <<"服务器端 文件重命名"<<"strOldPath: " << strOldPath << ", strNewPath"<< strNewPath;
        QDir dir;
        bool ret = dir.rename(strOldPath, strNewPath);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REPONSE;
        if(ret){ //重命名成功
            strcpy(respdu->caData, RENAME_FILE_OK);
        }else{ //重命名失败
            strcpy(respdu->caData, RENAME_FILE_FAILURED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:{
        char caName[32] = {'\0'};
        char *curPath = new char[pdu->uiMsgLen];
        strncpy(caName, pdu->caData,32);
        memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(curPath).arg(caName);
        QFileInfo fileInfo(strPath);
        PDU *respdu = NULL;
        if(fileInfo.isDir()){ // 是一个文件见路径 -> 获取当前文件目录下的所有信息
            QDir dir(strPath);
            QFileInfoList fileInfoList =  dir.entryInfoList(); // 以字符串列表的形式 返回 当前目录下的信息
            int iFileCount = fileInfoList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            qDebug() << "ENUM_MSG_TYPE_ENTER_DIR_REQUEST "<<"ENUM_MSG_TYPE_FRUSH_FILE_REPONSE";
            respdu->uiMsgType = ENUM_MSG_TYPE_FRUSH_FILE_REPONSE; // 消息类型不变, 可以使客户端直接刷新
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for(int i = 0; i < iFileCount; i++){
                pFileInfo = (FileInfo*)respdu->caMsg+i; // 指针+1, 代表地址上加上地址类型大小的数量, 将respdu->caMsg后面的地址转换为FileInfo*类型, 然后用一个地址变量指向他, 对这块地址进行操作
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size()); //向内存中拷贝名字, 之后需要判断文件类型
                if(fileInfoList[i].isDir()){ //判断文件类型, 是文件夹 还是文件, 或者是其他的什么类型的文件
                    pFileInfo->iFileType = 0;
                }else if(fileInfoList[i].isFile()){
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            pdu = NULL;
            break;
        }else if(fileInfo.isFile()){ //是一个文件
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REPONSE;
            qDebug() << "ENUM_MSG_TYPE_ENTER_DIR_REQUEST "<<"ENUM_MSG_TYPE_ENTER_DIR_REPONSE";
            strncpy(respdu->caData, ENTER_DIR_FAILURED, strlen(ENTER_DIR_FAILURED));
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        qDebug() << strPath <<"fileInfo.isDir()" <<fileInfo.isDir();
        qDebug() << strPath <<"fileInfo.isFile()" <<fileInfo.isFile();
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:{
        qDebug() <<"ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: 1";
        char caFileName[32] = {'\0'};
        qDebug() <<"ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: 2";
        qint64 fileSize = 0;
        qDebug() <<"ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: 3";
        sscanf(pdu->caData,"%s %lld", caFileName, &fileSize);
        qDebug() << "ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:" << caFileName <<"  " << fileSize;
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
        qDebug() <<" ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:  strPath " << strPath;
        delete []pPath;
        pPath = NULL;

        m_file.setFileName(strPath);
        //以只写的方式打开文件, 如果文件不存在,则会自动创建文件
        if(m_file.open(QIODevice::WriteOnly)){
            //可以改成多线程实现
            m_bUpload = true;
            m_iTotal = fileSize;
            m_iRecved = 0;
        }
        break;
    }
    case ENUM_MSG_TYPE_DEL_FILE_REQUEST:{
        char caName[32] = {'\0'}; // 拷贝名字
        strcpy(caName, pdu->caData); // caData为char类型
        char *pPath = new char[pdu->uiMsgLen];//拷贝路径
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);//拼接
        QFileInfo fileInfo(strPath);
        bool ret = false;
        if(fileInfo.isDir()){//判断是否是一个文件的路径
            ret = false; // 不删除常规文件, 只删除文件夹
        }else if(fileInfo.isFile()){//是一个常规文件}
            QDir dir;
            ret = dir.remove(strPath);
        }
        qDebug()<<"mytcpsocket :ENUM_MSG_TYPE_DEL_FILE_REQUEST" <<"isDir: "<<fileInfo.isDir()<<"isFile: " <<fileInfo.isFile()<<ret;
        PDU *respdu = NULL;
        if(ret){ //是一个文件夹, 可以删除, 删除成功
            respdu = mkPDU(strlen(DEL_FILE_OK)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REPONSE;
            memcpy(respdu->caData, DEL_FILE_OK, strlen(DEL_FILE_OK));
        }else{ // 是一个常规文件, 返回删除失败
            respdu = mkPDU(strlen(DEL_FILE_FAILURED)+1);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REPONSE;
            memcpy(respdu->caData, DEL_FILE_FAILURED, strlen(DEL_FILE_FAILURED));
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:{
        char caName[32] = {'\0'}; // 拷贝名字
        strcpy(caName, pdu->caData); // caData为char类型
        char *pPath = new char[pdu->uiMsgLen];//拷贝路径
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);
        qDebug() << "socket, " << " ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST" << strPath;
        delete []pPath;
        pPath = NULL;
        //先把客户要下载的文件的大小 发送给客户端
        QFileInfo fileInfo(strPath);
        qint64 fileSize = fileInfo.size();
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REPONSE;
        sprintf(respdu->caData, "%s %lld", caName, fileSize);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;

        m_file.setFileName(strPath);
        m_file.open(QIODevice::ReadOnly);
        m_pTimer->start(1000);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:{
        //依次将接收者的名字提取出来, 然后依次向接收者发送 信息
        //这个pdu中有两个 信息, 文件贡献发起人的 姓名, 文件地址
        char caSendName[32] = {"\0"};
        int num = 0;
        sscanf(pdu->caData, "%s %d", caSendName, &num);
        int size = num*32;
        PDU *respdu = mkPDU(pdu->uiMsgLen-size); // 这个pdu中可以把要共享的文件的路径传送回去, 这样接收者可以很轻松的知道要接收的文件是什么文件
        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;//在之后还可以把这个pdu直接传送回服务器,这样服务器也可以直接知道要传送给客户端的文件是哪一个文件, 直接传送就好了
        strcpy(respdu->caData, caSendName);
        memcpy(respdu->caMsg, (char*)pdu->caMsg+size, pdu->uiMsgLen-size);//拷贝路径
        qDebug() << "ENUM_MSG_TYPE_SHARE_FILE_REQUEST: respdu->caMsg-> " <<respdu->caMsg;
        char caRecvName[32] = {'\0'};
        for(int i = 0; i < num; i++){
            memcpy(caRecvName, (char*)(pdu->caMsg)+i*32, 32);
            MyTcpServer::getInstance().resend(caRecvName, respdu);
        }
        free(respdu);
        respdu = NULL;

        PDU *respdu2 = mkPDU(0);
        respdu2->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REPONSE;
        strcpy(respdu2->caData, "share file ok!");
        write((char*)respdu2, respdu2->uiPDULen);
        free(respdu2);
        respdu2 = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPONSE:{
        //默认将文件接收到 接收者 的底层目录 ./xxx(用户名)
        QString strRecvPath = QString("./%1").arg(pdu->caData);// 接收者的底层目录
        QString strShareFilePath = QString("%1").arg((char*)pdu->caMsg); // 分享的文件的路径
        int index = strShareFilePath.lastIndexOf('/');
        QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1); // 文件的名字
        strRecvPath = strRecvPath+'/'+strFileName; //新文件的保存路径
        QFileInfo fileInfo(strShareFilePath); //判断是文件夹 还是 普通文件
        qDebug() << "ENUM_MSG_TYPE_SHARE_FILE_REPONSE :fileInfo.isFile()? " <<fileInfo.isFile() << "ENUM_MSG_TYPE_SHARE_FILE_REPONSE :fileInfo.isDir()? " <<fileInfo.isDir();
        if(fileInfo.isFile()){ // 接收的文件还是要放在服务器端, 只不过是在另一个地方多了一个备份

            qDebug() << "strShareFilePath:" << strShareFilePath << " ; \n" << "strRecvPath:" <<strRecvPath;
            QFile::copy(strShareFilePath, strRecvPath);
        }else if(fileInfo.isDir()){
            copyDir(strShareFilePath, strRecvPath);
        }
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:{
        char caFileName[32] = {'\0'};
        int srcLen = 0;
        int destLen = 0;
        sscanf(pdu->caData,"%d%d%s", &srcLen, &destLen, caFileName);//从字符串中读取格式化输入
        char *pSrcPath = new char[srcLen+1];
        char *pDestPath = new char[destLen+1+32];//目的路径 加 目标文件的文件名的尺寸
        memset(pSrcPath, '\0', srcLen+1);
        memset(pDestPath, '\0', destLen+1);

        memcpy(pSrcPath, pdu->caMsg, srcLen);
        memcpy(pDestPath, (char*)(pdu->caMsg)+(srcLen+1), destLen);

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REPONSE;
        QFileInfo fileInfo(pDestPath);
        qDebug() << "server, ENUM_MSG_TYPE_MOVE_FILE_REQUEST:"<< pDestPath<< " isDir?" << fileInfo.isDir() << ", fileInfo.isFile() : " <<fileInfo.isFile();
        if(fileInfo.isDir()){ // 判断目的文件夹路径
            //是一个文件夹
           strcat(pDestPath, "/"); //将一个字符串连接到另一个字符串末尾
           strcat(pDestPath, caFileName);

           bool ret = QFile::rename(pSrcPath, pDestPath);
           if(ret){
                strcpy(respdu->caData, MOVE_FILE_OK);
           }else{
                strcpy(respdu->caData, COMMON_ERR);
           }
        }else if(fileInfo.isFile()){
            //是一个常规文件
            strcpy(respdu->caData, MOVE_FILE_OK);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu=NULL;
    }
    else{//是文件传输的消息的话, 进行接受接收
    //客户端上传文件的话, 会直接用二进制的形式传输过来, 不会封装到PDU中
    //接收上传的数据
        PDU *respdu = NULL;
        QByteArray buff = readAll(); //buff为接收到的数据
        m_file.write(buff); // 把数据写入到文件中
        m_iRecved += buff.size();
        if(m_iTotal == m_iRecved){
            m_file.close(); //文件关闭
            m_bUpload = false;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REPONSE;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }else if(m_iTotal < m_iRecved){ //总大小 < 已经收到的 -> 出错了
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILURED);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }

    }
}

    void MyTcpSocket::test()
    {
        qDebug() << " test worked";
    }

void MyTcpSocket::clientOffline()
{
     OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
     emit offLine(this); // 发送下线信号, 把TcpSocketList中对应的 socket删除掉
}

void MyTcpSocket::senFileToClient()
{
    m_pTimer->stop();
    char *pData = new char[4096];
    qint64 ret = 0;
    while(true){
        ret = m_file.read(pData, 4096);
        if(ret > 0 && ret <= 4096){
            write(pData, ret);
        }
        else if(0 ==ret){
            m_file.close();
            break;
        }
        else if(ret < 0){ // 出错的情况下
            qDebug() << "发送文件内容给客户端过程中失败, ret = " << ret;
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}
