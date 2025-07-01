#ifndef _AUTH_H_
#define _AUTH_H_
/*
该代码表示验证部分 通过串口5 进行身份验证
验证通过后调用shell脚本下载数据

*/
#include "uart/uart.h"
extern int send_data(const char *uart_name,char *uart_send,unsigned char uart_send_size);
void* auth_init(void *Pdata);

#endif