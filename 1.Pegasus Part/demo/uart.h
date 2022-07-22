#ifndef UART_H
#define UART_H

#define UART_BUFF_SIZE           128
#define WRITE_BY_INT
#define UART_DEMO_TASK_STAK_SIZE 4096
#define UART_DEMO_TASK_PRIORITY  25
#define DEMO_UART_NUM            HI_UART_IDX_1

typedef enum {
    UART_RECEIVE_FLAG = 0,
    UART_RECVIVE_LEN,
    UART_SEND_FLAG = 2,
    UART_SEND_LEN
}UartDefType;

typedef enum {
    UART_RECV_TRUE = 0,
    UART_RECV_FALSE,
}UartRecvDef;

typedef struct {
    unsigned int uartChannel;
    unsigned char g_receiveUartBuff[UART_BUFF_SIZE];
    int g_uartReceiveFlag;
    int g_uartLen;
}UartDefConfig;

int SetUartRecvFlag(UartRecvDef def);
int GetUartConfig(UartDefType type);
void ResetUartReceiveMsg(void);
unsigned char *GetUartReceiveMsg(void);

/*初始化串口*/
void InitUart(void);
void UartTransmit(void);
void UartDemoTask2();
#endif