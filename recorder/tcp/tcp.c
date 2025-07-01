
#include "tcp.h"
#include "crc/crc32.h"
#include "alg/pp_buff.h"
#include <errno.h>


#define SERVER_IP "192.168.1.228"
#define SERVER_PORT 8089
#define TYPE_SIZE 512
#define TCP_SIZE 72

#define DEBUG 1

int client_fd=-1;
static unsigned char head_type[2] = {0x5A,0x5A};
static int head_index=0;

static ppbuf_t *p=NULL;

unsigned int GetDword(void *Pdata,unsigned int offset)
{
	return *((unsigned int*)((unsigned char *)Pdata+offset));
};

void *receive(void *pdata){
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
        ret=recv(client_fd,recvbuff,TYPE_SIZE,0);
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
                                printf("throw tcp type  \n");
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


void* tcp_init(void *Pdata){
    struct sockaddr_in dst_addr;
    int ret=-1;
    pthread_t pid;

    //char buff[TYPE_SIZE]="TCP SEND MSG TEST \n";
    p=(ppbuf_t *)Pdata;

    client_fd=socket(AF_INET,SOCK_STREAM,0);
    if(client_fd<0){
        printf("create socket err \n");
        return NULL;
    }
    memset(&dst_addr,0,sizeof(dst_addr));
    dst_addr.sin_family=AF_INET;
    dst_addr.sin_port= htons(SERVER_PORT);
    dst_addr.sin_addr.s_addr =  inet_addr(SERVER_IP); 
    
    ret = connect(client_fd, (const struct sockaddr *)&dst_addr,sizeof(dst_addr));
    if(ret<0){
        printf("bind socket err \n");
        return NULL;
    }
    ret = pthread_create(&pid,NULL,receive,NULL);
    if(ret<0){
        printf("pthread create err \n");
        return NULL;
    }
    pthread_detach(pid); // 自动回收线程资源

    while(1);
    return 0;
}