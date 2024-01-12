#include"protool.h"

//产生一个协议控制单元, make protool data unit();
//协议 动态申请空间
PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU) + uiMsgLen;
    PDU* pdu = (PDU*)malloc(uiPDULen);
    if(NULL == pdu){
        exit(EXIT_FAILURE);
    }
    memset(pdu, 0, uiPDULen);
    //void *memset(void *s , int ch , size_t  n ), 将s中的前n个字节用ch替换并且返回s | 将数字以单个字节逐个拷贝的方式放到指定的内存中去
    //用于快速初始化大片内存
    pdu -> uiPDULen = uiPDULen;
    pdu -> uiMsgLen = uiMsgLen;
    return pdu;
}
