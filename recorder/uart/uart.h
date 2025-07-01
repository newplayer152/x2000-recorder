#ifndef _UART_H_
#define _UART_H_
/*
串口数据的读取与封装成数据包

*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "ingenic_uart.h"
#include "tcp/tcp.h"
#include <semaphore.h>
#include <pthread.h>



extern unsigned int GetDword(void *Pdata,unsigned int offset);

extern pthread_rwlock_t g_rwlock;//声明读写锁
extern sem_t g_sem;


void* uart_init(void *Pdata);

#endif