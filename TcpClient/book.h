#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include<QListWidget> //显示文件的列表
#include<QPushButton> // 操作文件的选项
#include<QHBoxLayout> //操作布局
#include<QVBoxLayout>
#include<protool.h>
#include<QTimer>

class Book : public QWidget //此处的Book类就是文件操作界面相关的book类
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    void clearEnterDir();
    QString enterDir();
    void setDownloadFlag(bool status);
    bool getDownloadStatus();
    QString getSaveFilePath();
    QString getShareFileName();

    qint64 m_iTotal; //下载的时候: 总的文件的大小
    qint64 m_iRecved;//下载的时候 已经接收的文件的大小
signals:

public slots:
    void createDir();
    void frushFile();
    void delDir();
    void renameFile();
    void enterDir(QListWidgetItem *item);
//    void onItemDoubleClicked(QListWidgetItem *item);


    void returnPre();
    void uploadFile();

    void uploadFileData();
    void delRegFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDestDir();

private:
    QListWidget *m_pBookListW; //用来显示文件名的的列表
    QPushButton *m_pReturnPB;//操作文件的按钮 一共9个
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB; //删除路径/文件夹
    QPushButton *m_pRenamePB;
    QPushButton *m_pFrushPB;
    QPushButton *m_puploadPB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pDelFilePB; //删除文件
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;

    QPushButton *m_pSelectDirPB;//移动文件的时候, 选择目标文件夹路径的按钮

    QString m_strEnterDir;
    QString m_strUploadFilePath; // 字符串变量, 用来保存打开的路径

    QTimer *m_pTimer;//定时器对象 指针

    QString m_strSaveFilePath;
    bool m_bDownload; // 判断是否正处于下载的状态
    QString m_strShareFileName;

    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
};

#endif // BOOK_H
