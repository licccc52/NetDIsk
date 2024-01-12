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
    pdu -> uiPDULen = uiPDULen;
    pdu -> uiMsgLen = uiMsgLen;
    return pdu;
}
