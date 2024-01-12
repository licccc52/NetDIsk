#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include<QListWidget>
#include"friend.h"
#include"book.h"
#include<QStackedWidget>



class OpeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpeWidget(QWidget *parent = nullptr);
    static OpeWidget &getInstance();
    Friend* getFriend();
    Book *getBook();

signals:

private:
    QListWidget* m_pListW;
    Friend *m_pFriend; //用于操作好友的界面
    Book *m_pBook;  //用于操作书籍的界面

    QStackedWidget* m_pSW;
};

#endif // OPEWIDGET_H
