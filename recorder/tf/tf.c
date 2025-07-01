
#include "tf.h"
#include "alg/pp_buff.h"


char ss[50];


int sd_mount(void){
	int ret =-1;
	ret = mount("/dev/mmcblk2p1","/mnt/sdcard/","vfat",0,NULL);
	if(ret == 0){
		return 0;
	}else{
		perror("mount err\n");
		return -1;
	}
}

void sd_unmount(void){
	int ret =-1;
	ret = umount2("/mnt/sdcard/", MNT_DETACH);
	if (ret == 0) {
		printf("/sdcard remove sucessful!\n");
	}
}






int tf_init(void)
{
	int ret = -1;
	int rtc_fd=-1;
	struct rtc_time rtc_tm;
	FILE *fp;
	
	mkdir("/mnt/sdcard/",0777);
	sd_unmount();
	ret = sd_mount();
	if(ret == -1){
		return -1;
	}

	rtc_fd=open("/dev/rtc0",O_RDWR);
    if(rtc_fd<=0){
        perror("open rtc fail!!\n");
        exit(errno);
    }
	ret=ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
    if(ret<0){
        perror("get rtc date fail!!\n");
        exit(errno);
    }
	close(rtc_fd);

    //sprintf("get rtc time %d/%d/%d %d:%d:%d\n",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday,rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);
	sprintf(ss,"/mnt/sdcard/%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d.txt", (1900+rtc_tm.tm_year),(1+rtc_tm.tm_mon), rtc_tm.tm_mday, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
	printf("file is : %s \n", ss);
	fp = fopen(ss,"w+");
	if(fp == NULL){
		perror("fopen err");
		return -1;
	}

	fflush(fp);
	fclose(fp);

	return 0;
}
#define BUFF_MAXLEN (72*60*1)//一次性存储60个包
void *data_storeage(void *Pdata){
	FILE *fp;
    unsigned char rd_buff[BUFF_MAXLEN];
    ppbuf_t *p = (ppbuf_t *)Pdata;
	while(1){
		sem_wait(&g_sem); 
        printf("file is : %s\n",ss);
        fp=fopen(ss,"a+");
		if(fp == NULL)
        {
            perror("open txt err");
            exit(errno);
        }
        printf("------------write---------\n");
        pthread_rwlock_rdlock(&g_rwlock);
        ppbuf_get_pong(p, &rd_buff, BUFF_MAXLEN);
        printf("get_index %d \n",p->get_index);
		fwrite(rd_buff,BUFF_MAXLEN,1,fp);
		fflush(fp);
		//解锁
		pthread_rwlock_unlock(&g_rwlock);
		fclose(fp);
		
	}



}