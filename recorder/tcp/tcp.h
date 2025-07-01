#ifndef _TCP_H_
#define _TCP_H_
/*
网口的初始化 网口的数据记录
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <semaphore.h>



extern pthread_rwlock_t g_rwlock;//声明读写锁

extern sem_t g_sem;

void* tcp_init(void *Pdata);

#endif