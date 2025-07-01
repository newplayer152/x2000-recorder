
#include "alg/pp_buff.h"
#include "sadc/sadc.h"
#include "tf/tf.h"
#include "tcp/tcp.h"
#include "uart/uart.h"
#include "auth/auth.h"

/*

主循环开启 tf、sadc、网口、串口5、串口6等数据采集线程


*/


#define BUFF_MAXLEN (72*60*1)

PPBUF_DECLEAR(acer,BUFF_MAXLEN);


pthread_rwlock_t g_rwlock;//读写锁用来避免pingpongbuffer的资源访问冲突
sem_t g_sem; //这个信号用来通知tf中的写入操作，可由各个数据采集函数发送

int main(){
    int ret = -1; 
    pthread_t pid1;
    pthread_t pid2;
    pthread_t pid3;
    pthread_t pid4;
    pthread_t pid5;
    //挂载tf卡并创建文件
    tf_init();

    //初始化信号量
    sem_init(&g_sem, 0, 0);

    //初始化读写锁
    pthread_rwlock_init(&g_rwlock,NULL);

    //创建SADC数据采集线程
    ret = pthread_create(&pid1,NULL,sadc_init,&acer); 
    if(ret <0)
    {
        printf("pthread creat err \n");
        return  -1;
    }
    //创建TF卡数据存储线程
    ret = pthread_create(&pid2,NULL,data_storeage,&acer); 
    if(ret <0)
    {
        printf("pthread creat err \n");
        return  -1;
    }
    //创建tcp数据采集线程
    ret = pthread_create(&pid3,NULL,tcp_init,&acer); 
    if(ret <0)
    {
        printf("pthread creat err \n");
        return  -1;
    }

    //创建uart6数据采集线程
    ret = pthread_create(&pid4,NULL,uart_init,&acer); 
    if(ret <0)
    {
        printf("pthread creat err \n");
        return  -1;
    }

    //创建uart5认证与下载线程
    ret = pthread_create(&pid5,NULL,auth_init,&acer); 
    if(ret <0)
    {
        printf("pthread creat err \n");
        return  -1;
    }
    pthread_join(pid1,NULL);
    pthread_join(pid2,NULL);
    pthread_join(pid3,NULL);
    pthread_join(pid4,NULL);
    pthread_join(pid5,NULL);
    
    while(1);

}