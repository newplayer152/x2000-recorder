#ifndef SADC_H_
#define SADC_H_
/*
每秒读取sadc口的数据并封装成数据包，每分钟存入一次磁盘

*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <semaphore.h>
#include <pthread.h>
#include "alg/pp_buff.h"
#include "crc/crc32.h"



typedef struct ad_record{
	unsigned short head;
	unsigned short FrameID;
	unsigned short SrcType;
	unsigned short DataLen;
	float Chn1;
	float Chn2;
	float Chn3;
	float Chn4;
	float Chn5;
	float Chn6;
	struct rtc_time rtc_tm;
	unsigned int crc32;
}ad_record_t;


extern pthread_rwlock_t g_rwlock;//声明读写锁
extern sem_t g_sem;

void *sadc_init(void *Pdata);

#endif