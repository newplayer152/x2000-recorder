
#include "auth.h"
#include "hash/hash.h"
#include <errno.h>


#define AUTH_HASH_SIZE 20
#define KEY_MAX_LEN 64

#define DEBUG 1



/**
 * @brief  发送, 接收数据测试
 * @param  uart_name: 输入参数, 串口名
 * @return 成功: 0
 *         失败: -1
 */





void *auth_init(void *Pdata)
{
    int ret = -1;
    const char *uart5 = "/dev/ttyS5";
    pthread_t pid;
    FILE *fp;
    ret = open_uart(uart5, BAUD_115200, 8, 'n', 1);
    if (ret < 0) {
        perror("open uart5 fail\n");
        return NULL;
    }

    if (ret < 0) {
        perror("pthread create fail\n");
        return NULL;
    }

    unsigned char recvbuff[AUTH_HASH_SIZE];
    unsigned char dealbuff[AUTH_HASH_SIZE];
    unsigned char hashed_buff[KEY_MAX_LEN];
    unsigned char key[KEY_MAX_LEN];
    int deal_index=0;
    int dealbuff_statu=0;


    unsigned char * ptr=NULL;

    memset(recvbuff,0,AUTH_HASH_SIZE);
    memset(dealbuff,0,AUTH_HASH_SIZE);
    memset(hashed_buff,0,KEY_MAX_LEN);

    fp = fopen("/mnt/sdcard/key.txt","r");
    //fread(fp,key2,64);
        // 使用 fseek 指向文件首地址
    fseek(fp, 0, SEEK_SET);
    fread(key, KEY_MAX_LEN , 1, fp );
    printf("%s\n",key);
    send_data(uart5, "please 8-20 log key press enter to over",40); 
    while(1){
        ret = uart_read_data("/dev/ttyS5", recvbuff, AUTH_HASH_SIZE, 3);//超时时间 3ms
        if(ret<=0){//没有数据可读
            continue;
        }
        switch (dealbuff_statu){
            case 0://未接收完
                {
                    if(ret + deal_index > AUTH_HASH_SIZE){//密钥超限
                        memset(recvbuff,0,AUTH_HASH_SIZE);//重置
                        memset(dealbuff,0,AUTH_HASH_SIZE);
                        memset(hashed_buff,0,KEY_MAX_LEN);
                        dealbuff_statu=0;
                        ptr=NULL;
                        deal_index=0;
                        continue;
                    }else{
                        ptr=strchr(recvbuff,'\n');
                        if(ptr!=NULL){
                            memcpy((dealbuff + deal_index),recvbuff,ptr - recvbuff );//只取出\n前面数据
                            dealbuff_statu=1; //已接收完
                        }else{
                            memcpy((dealbuff + deal_index),recvbuff,ret);
                        }
                        
                    }
                    
                    break;
                }
            case 1://接收完了开始处理
                {
                    hash(dealbuff, hashed_buff);//hash运算
                    if(0 == memcmp(hashed_buff,key,KEY_MAX_LEN)){

                        printf("log is ok \n");
                        system("/mnt/sdcard/download.sh");
                    }else printf("passward is err \n");

                    memset(recvbuff,0,AUTH_HASH_SIZE);//重置
                    memset(dealbuff,0,AUTH_HASH_SIZE);
                    memset(hashed_buff,0,KEY_MAX_LEN);
                    dealbuff_statu=0;
                    ptr=NULL;
                    deal_index=0;

                    break;
                }

            default:
                break;
        }
            

    }

    return 0;

}
