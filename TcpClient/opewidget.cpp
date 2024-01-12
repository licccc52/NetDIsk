#include "opewidget.h"


OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    m_pListW = new QListWidget(this);//显示可选择列表的控件
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;

    m_pSW = new QStackedWidget; // QStackedWidget() 用于管理多个子部件, 但是一次只能显示一个, 类似卡片堆叠, 只显示堆叠中的当前部件, 其他部件被隐藏
    m_pSW->addWidget(m_pFriend);  // 可以通过索引或者特定标识符来显示哪个子部件, 其他部件会被自动隐藏
    m_pSW->addWidget(m_pBook);  // 向栈里面加入元素

    QHBoxLayout* pMain = new QHBoxLayout; // 调整布局
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);

    connect(m_pListW, SIGNAL(currentRowChanged(int)), m_pSW, SLOT(setCurrentIndex(int)));
    //currentRowChanged()是一个信号函数, 在部件当前项改变时,currentRowChanged()信号会被发射 'emitted',这个信号会告诉连接到
    //它的槽函数当前的选择发生了变化, 参数表示的意思是 用户当前选择的 '行数'

}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance; // 产生了一个静态的操作界面的对象, 无论调用多少次, 调用的都是同一个对象;
    return instance;

}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
