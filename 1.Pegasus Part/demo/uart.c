/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include <hi_stdlib.h>
#include <hisignalling_protocol.h>
#include <hi_uart.h>
#include <uart.h>
#include <iot_uart.h>
#include <hi_gpio.h>
#include <hi_io.h>
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "pid.h"

UartDefConfig uartDefConfig = {0};
extern int uart_flag;
extern unsigned char* uart_data[15];

static void Uart1GpioCOnfig(void)
{
    IoSetFunc(HI_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_UART1_TXD);
    IoSetFunc(HI_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_UART1_RXD);
}

/*设定接收完毕标志位*/
int SetUartRecvFlag(UartRecvDef def)
{
    if (def == UART_RECV_TRUE) {
        uartDefConfig.g_uartReceiveFlag = HI_TRUE;
    } else {
        uartDefConfig.g_uartReceiveFlag = HI_FALSE;
    }
    
    return uartDefConfig.g_uartReceiveFlag;
}

/*获取串口配置信息*/
int GetUartConfig(UartDefType type)
{
    int receive = 0;

    switch (type) {
        case UART_RECEIVE_FLAG:
            receive = uartDefConfig.g_uartReceiveFlag;
            break;
        case UART_RECVIVE_LEN:
            receive = uartDefConfig.g_uartLen;
            break;
        default:
            break;
    }
    return receive;
}

void ResetUartReceiveMsg(void)
{
    (void)memset_s(uartDefConfig.g_receiveUartBuff, sizeof(uartDefConfig.g_receiveUartBuff),
        0x0, sizeof(uartDefConfig.g_receiveUartBuff));
}

unsigned char *GetUartReceiveMsg(void)
{
    return uartDefConfig.g_receiveUartBuff;
}

static hi_void *UartDemoTask(char *param)
{
    hi_u8 uart_buffer[32];
    hi_u8 data[22];
    hi_unref_param(param);
    printf("Initialize uart demo successfully, please enter some datas via DEMO_UART_NUM port...\n");
    Uart1GpioCOnfig();
    for (;;) {
        uartDefConfig.g_uartLen = IoTUartRead(DEMO_UART_NUM, uart_buffer, UART_BUFF_SIZE);
        if (uartDefConfig.g_uartLen > 0 && uart_flag == 0) {
            if (GetUartConfig(UART_RECEIVE_FLAG) == HI_FALSE) {
                (void)memcpy_s(uartDefConfig.g_receiveUartBuff, uartDefConfig.g_uartLen,
                    uart_buffer, uartDefConfig.g_uartLen);
                (void)SetUartRecvFlag(UART_RECV_TRUE);

                ExtractData(uart_buffer, uart_data);
                uart_flag = 1;
            }
        }
        TaskMsleep(1000); /* 20:sleep 20ms */
    }
    return HI_NULL;
}


void InitUart(void){
    //配置串口协议
    IotUartAttribute uartAttr = {
        .baudRate = 115200, /* baudRate: 115200 */
        .dataBits = 8, /* dataBits: 8bits */
        .stopBits = 1, /* stop bit */
        .parity = 0,
    };
    //初始化串口设备*/
    IoTUartInit(DEMO_UART_NUM, &uartAttr);
}

/*
 * This demo simply shows how to read datas from UART2 port and then echo back.
 */
void UartTransmit(void)
{
    //启用新线程通讯
    osThreadAttr_t attr = {0};
    attr.stack_size = UART_DEMO_TASK_STAK_SIZE;
    attr.priority = UART_DEMO_TASK_PRIORITY;
    attr.name = (hi_char*)"uart demo";
    if (osThreadNew((osThreadFunc_t)UartDemoTask, NULL, &attr) == NULL) {
        printf("Falied to create uart demo task!\n");
    }
}