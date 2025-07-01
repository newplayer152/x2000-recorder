
#include "uart.h"
#include "alg/pp_buff.h"
#include "crc/crc32.h"
#include <errno.h>

#define TIMEOUT 1000
#define TYPE_SIZE 512
#define TCP_SIZE 72
#define DEBUG 1

static unsigned char head_type[2] = {0x5A,0x5A};
static int head_index=0;

static ppbuf_t *p=NULL;

/**
 * @brief  发送, 接收数据测试
 * @param  uart_name: 输入参数, 串口名
 * @return 成功: 0
 *         失败: -1
 */
int send_data(const char *uart_name,char *uart_send,unsigned char uart_send_size)
{
    int ret = -1;

    // uint8_t uart_send[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    // uint32_t uart_send_size = 7;

    // printf("uart %s write data[len = %d]: ", uart_name, uart_send_size);
    // for (uint8_t i = 0; i < uart_send_size; i++)
    //     printf("0x%02X ", uart_send[i]);
    // printf("\n");

    ret = uart_write_data(uart_name, uart_send, uart_send_size);
    if (ret < 0) {
        printf("uart %s send data fail\n", uart_name);
        close_uart(uart_name);
        return -1;
    }

    ret = uart_clear_data(uart_name);
    if (0 != ret) {
        printf("uart %s clear data fail\n", uart_name);
        close_uart(uart_name);
        return -1;
    }
    return 0;
}


void *CallBackUartt6_rcv(void *Pdata){
    unsigned char recvbuff[TYPE_SIZE];
    unsigned char dealbuff[TYPE_SIZE];
    int recv_index=-1;
    int deal_index=0;
    int dealbuff_statu=0;
    int ret=-1;
    unsigned int getCRC32=0;
    unsigned int calCRC32=0;
    while(1){
        memset(recvbuff,0,TYPE_SIZE);
        ret = uart_read_data("/dev/ttyS6", recvbuff, TCP_SIZE, -1);//要无超时时间才能快速接受数据
        if(ret<=0){//没有数据可读
            continue;
        }
        
        while(1){
            recv_index++;
            switch (dealbuff_statu)
            {
                case  0://检测头
                    {
                        if(recvbuff[recv_index]==head_type[head_index]){
                            head_index++;
                            if(head_index>=2){
                                dealbuff[deal_index++] = head_type[0];
                                dealbuff[deal_index++] = head_type[1];
                                dealbuff_statu=1;//一包开始
                                head_index=0;//检测头标志重置
                            }
                        }else{
                            dealbuff_statu=0;
                            head_index=0;
                        }
                        break;
                    }
                    
                case  1://已经检测到头正在处理
                    {
                        dealbuff[deal_index++]=recvbuff[recv_index];
                        if(deal_index>=TCP_SIZE){
                            if(DEBUG){//调试查看输出
                                int enter=10;
                                for(int i=0;i<deal_index;i++){
                                    enter--;
                                    printf("%02x ",dealbuff[i]);
                                    if(enter==0){
                                        printf("\n");
                                        enter=10;
                                    }
                                }
                            }
                            
                            getCRC32=GetDword(dealbuff,68);
                            calCRC32=crc32(dealbuff,68);
                            if(getCRC32==calCRC32){//校验成功开始存储
                                //加锁
                                pthread_rwlock_wrlock(&g_rwlock);
                                if(ppbuf_get_full_signal(p,false)!=true){
                                    printf("put_index %d \n", p->put_index);
                                    ppbuf_put_ping(p, (unsigned char*)&dealbuff, TCP_SIZE);//把dealbuff当作unsigned char写进去
                                    
                                }else{
                                    printf("---------full-------\n");
                                    ppbuf_get_full_signal(p,true);//满了交换buff

                                    printf("put_index %d \n", p->put_index);//换完了buff再写数据 不丢包
                                    ppbuf_put_ping(p, (unsigned char*)&dealbuff, TCP_SIZE);//把dealbuff当作unsigned char写进去

                                    int ret = sem_post(&g_sem);//发送信号让tf模块取读数据
                                    if(ret !=0)
                                    {
                                        pthread_rwlock_unlock(&g_rwlock);
                                        perror("sem_post err");
                                        exit(errno);
                                    }
                                }
                                pthread_rwlock_unlock(&g_rwlock);
                            }else{
                                printf("throw uart type  \n");
                            }

                            dealbuff_statu=0;//一包结束 清理标志与存储buff 
                            memset(dealbuff,0,TYPE_SIZE);
                            deal_index=0;                       
                        }
                        break;
                    }
                default:
                    break;
            }
            if(recv_index>=ret){//读完了处理边界
                recv_index=-1;//重置
                break;
            }
        }
    }
    

}

void *uart_init(void *Pdata)
{
    int ret = -1;
    const char *uart6 = "/dev/ttyS6";
    //char *uart6_buff="hellow";

    pthread_t pid;

    p=(ppbuf_t *)Pdata;



    ret = open_uart(uart6, BAUD_115200, 8, 'n', 1);
    if (ret < 0) {
        perror("open uart6 fail\n");
        return NULL;
    }
    ret = pthread_create(&pid,NULL,CallBackUartt6_rcv,NULL);
    if (ret < 0) {
        perror("pthread create fail\n");
        return NULL;
    }


    while(1){
        // if(DEBUG){//调试查看输出
            // send_data(uart6,uart6_buff,strlen(uart6_buff));
            // sleep(1);
        // }
        
    }

    return 0;

}
