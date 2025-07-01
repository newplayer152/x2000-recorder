#ifndef _TF_H
#define _TF_H
/*
    用来存储每分钟各个数据到磁盘的数据


*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <linux/rtc.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

int sd_mount(void);

void sd_unmount(void);

int tf_init(void);//挂载tf 获取系统时间建立TXT文件

void *data_storeage(void *Pdata);//存入数据

extern pthread_rwlock_t g_rwlock;
extern sem_t g_sem;

#endif
