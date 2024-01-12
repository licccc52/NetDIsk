#ifndef FRIEND_H
#define FRIEND_H

#include"online.h"
#include "protool.h"
#include<QDebug>
#include <QWidget>
#include<QTextEdit>
#include<QListWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>



class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU* pdu);
    void updateFriendList(PDU *pdu);
    void updateGroupMessage(PDU *pdu);

    QString m_strSearchName;

    QListWidget *getFriendList();

public slots:
    void showOnline();
    void searchUsr();
    void frushFriend();
    void deleteFriend();
    void privateChat();
    void groupChat();

signals:

private:
    QTextEdit* m_pShowMsgTE;
    QListWidget* m_pFriendListWidget;
    QLineEdit* m_pInputMsgLE;

    QPushButton* m_pDelFriendPB;
    QPushButton* m_pFlushFriendPB;
    QPushButton* m_pShowOnlineUsrPB;
    QPushButton* m_pSearchUsrPB;
    QPushButton* m_pMsgSendPB;
    QPushButton* m_pPrivateChatPB ;
    Online* m_pOnline; //online界面的对象

};

#endif // FRIEND_H
