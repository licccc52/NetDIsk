#ifndef PROTOOL_H
#define PROTOOL_H

#include<stdlib.h>
#include<unistd.h>
#include<string.h>

//弹性结构体
typedef unsigned int uint;
#define REGIST_OK "regist complete"
#define REGIST_FAILED "regist failed: name existed"
#define LOGIN_OK "Login success!"
#define LOGIN_FAILED "Login failed: name or pwd error!"

#define SEARCH_USR_NO "NO SUCH PEOPLE"
#define SEARCH_USR_ONLINE "ONLINE"
#define SEARCH_USR_OFFLINE "OFFLINE"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "existed friend"
#define ADD_FRIEND_OFFLINE "user offline"
#define ADD_FRIEND_NOEXIST "user not existed"
#define DIR_NO_EXIST "current dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define CREATE_DIR_OK "create dir ok"
#define DEL_DIR_OK "delete dir ok"
#define DEL_DIR_FAILURED "delete dir failured: is regular file"
#define RENAME_FILE_OK "rename file ok"
#define RENAME_FILE_FAILURED "rename file failured"
#define ENTER_DIR_FAILURED "ENTER DIR FAILURED: is a regular file"
#define DEL_FRIEND_OK "delete friend success!"
#define UPLOAD_FILE_OK "Upload file OK"
#define UPLOAD_FILE_FAILURED "Upload file failured"
#define DEL_FILE_OK "delete file ok"
#define DEL_FILE_FAILURED "delete file failured: is a directory"

#define MOVE_FILE_OK "move file ok"
#define MOVE_FILE_FAILURED "move file failured: is regular file"

#define COMMON_ERR "operate failed: system is busy"
enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,
    ENUM_MSG_TYPE_LOGIN_REQUEST, // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND, // 登录回复
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,// 查找所有在线用户 请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND, // 查找所有在线用户 回复
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST, // 搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND, // 搜索用户回复
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST, //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND, //添加好友回复
    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,  //添加好友同意
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE, //添加好友拒绝
    ENUM_MSG_TYPE_FRUSH_FRIEND_REQUEST, //刷新好友请求
    ENUM_MSG_TYPE_FRUSH_FRIEND_RESPOND, //刷新好友回复
    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST, //删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND, //删除好友回复
    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST, // 私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_REPONSE, //私聊回复
    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST, // 群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_REPONSE, //群聊回复
    ENUM_MSG_TYPE_CREATE_DIR_REQUEST, // 创建文件夹的请求
    ENUM_MSG_TYPE_CREATE_DIR_REPONSE, //创建文件夹的回复
    ENUM_MSG_TYPE_FRUSH_FILE_REQUEST, //刷新文件请求
    ENUM_MSG_TYPE_FRUSH_FILE_REPONSE, //刷新文件回复
    ENUM_MSG_TYPE_DEL_DIR_REQUEST, // 删除目录请求
    ENUM_MSG_TYPE_DEL_DIR_REPONSE, // 删除目录回复
    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,//重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_REPONSE,//重命名文件回复
    ENUM_MSG_TYPE_ENTER_DIR_REQUEST, //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_REPONSE, //进入文件夹回复
    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,//上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_REPONSE, //上传文件回复
    ENUM_MSG_TYPE_DEL_FILE_REQUEST, //删除常规文件请求
    ENUM_MSG_TYPE_DEL_FILE_REPONSE,//删除常规文件回复
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REPONSE,//下载文件回复
    ENUM_MSG_TYPE_SHARE_FILE_REQUEST, //共享文件请求
    ENUM_MSG_TYPE_SHARE_FILE_REPONSE, //共享文件回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTE, //共享文件的通知
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPONSE,//客户端愿意接收贡献文件

    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,//移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_REPONSE,//移动文件回复
    ENUM_MSG_TYPE_MAX = 0x00ffffff,
};

struct FileInfo{
    char caFileName[32]; // 文件名字
    int iFileType; // 文件类型
};

struct PDU{   // 协议数据单元
    uint uiPDULen; // 总的协议数据单元大小
    uint uiMsgType; // 消息类型
    char caData[64];
    uint uiMsgLen; // 实际消息长度
    int caMsg[];   // 实际消息
};


PDU* mkPDU(uint uiMsgLen);
#endif // PROTOOL_H
