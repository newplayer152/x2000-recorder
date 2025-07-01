#ifndef PP_BUFF_H
#define PP_BUFF_H

/*

    本代码实现数据的乒乓buffer的数据存取队列




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
#include <stdbool.h>

typedef struct {
    unsigned char *buffer_data;
    unsigned char ping;
    unsigned char pong;
    int buffer_size;
    int put_index;
    int get_index;
    bool full_signal;

}ppbuf_t;


int ppbuf_put_ping(ppbuf_t *p, void *data, int size);//把数据存放在ping队列中
int ppbuf_get_pong(ppbuf_t *p, void *data, int size);//把数据从pong队列中取出
unsigned char *ppbuf_get_addr(ppbuf_t* p, int size);
bool ppbuf_get_full_signal(ppbuf_t *p, bool consume);//判断是否写满 以及通过consume选择是否交换ping pong


//宏定义PPBUF_DECLEAR实现ping pong buffer的初始化
#define PPBUF_DECLEAR(name,size)                     \
        unsigned char ppbuff_mem_##name[size*2] ={0}; \
        ppbuf_t name ={                              \
            .buffer_data=&ppbuff_mem_##name[0],     \
            .ping=1,                               \
            .pong=0,                                \
            .buffer_size=size,                    \
            .put_index=0,                           \
            .get_index=0,                          \
            .full_signal=false                     \
        };                                          
#endif