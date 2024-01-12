#ifndef ONLINE_H
#define ONLINE_H

#include "protool.h"

#include <QWidget>

namespace Ui {
class Online;   //此命名空间用于阻止UI相关的类和对象
}

class Online : public QWidget
{
    Q_OBJECT // Qt中用于支持元对象特性的宏,它必须声明在包含信号和槽的类中使用,以便使得Qt的元对象系统能够识别该类中的信号和槽

public:
    explicit Online(QWidget *parent = nullptr);
    void showUsr(PDU* pdu);
    ~Online();

private slots:
    void on_addFriend_clicked();

private:
    Ui::Online *ui;  // 此指针可以用于访问和操作与 Online 类关联的所有UI元素
};

#endif // ONLINE_H
