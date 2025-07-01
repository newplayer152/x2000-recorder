
#include "sadc.h"

#define ADC_MAGIC_NUMBER	'A'
#define ADC_ENABLE			_IO(ADC_MAGIC_NUMBER, 11)
#define ADC_DISABLE			_IO(ADC_MAGIC_NUMBER, 22)
#define ADC_SET_VREF		_IOW(ADC_MAGIC_NUMBER, 33, unsigned int)

static int fd,fd1,fd2,fd3,fd4,fd5;

static int wdt_fd=-1;
static ppbuf_t *p = NULL;
static ad_record_t ADCTxData; 
static unsigned short FrameID = 0;
void adc_get_record(int signum){


	int data;
	int rtc_fd=-1,rtc_ret=-1;
	
	float f_addata1 = 0,f_addata2 = 0,f_addata3 = 0;
	float f_addata4 = 0,f_addata5 = 0,f_addata6 = 0;

	struct rtc_time rtc_tm;
	rtc_fd=open("/dev/rtc0",O_RDWR);
    if(rtc_fd<=0){
        perror("open rtc fail!!\n");
        exit(errno);
    }
	rtc_ret=ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm);
    if(rtc_ret<0){
        perror("get rtc date fail!!\n");
        exit(errno);
    }
	close(rtc_fd);

	printf("get record time %d/%d/%d %d:%d:%d\n",rtc_tm.tm_year+1900,rtc_tm.tm_mon+1,rtc_tm.tm_mday,rtc_tm.tm_hour,rtc_tm.tm_min,rtc_tm.tm_sec);

	read (fd, (char *)&data, sizeof(data));
	f_addata1 = data*10/1000;
	read (fd1, (char *)&data, sizeof(data));
	f_addata2 = data*10/1000.f;	
	read (fd2, (char *)&data, sizeof(data));
	f_addata3 = data*10/1000.f;	
	read (fd3, (char *)&data, sizeof(data));
	f_addata4 = data*10/1000.f;	
	read (fd4, (char *)&data, sizeof(data));
	f_addata5 = data*10/1000.f;
	read (fd5, (char *)&data, sizeof(data));
	f_addata6 = data*10/1000.f;
	
	// printf("reading : %d %d %d %d %d %d\n",f_addata1,f_addata2,f_addata3,f_addata4,f_addata5,f_addata6);

	//数据包赋值
	ADCTxData.head    = 0xa5a5;
    ADCTxData.SrcType = 0x0001;
    ADCTxData.DataLen = 0x48; //72
    ADCTxData.FrameID =  FrameID;
	FrameID++;
	if(FrameID == 65536)
	{
	 FrameID = 0;  
	}
	ADCTxData.Chn1 = f_addata1;
	ADCTxData.Chn2 = f_addata2;
	ADCTxData.Chn3 = f_addata3;
	ADCTxData.Chn4 = f_addata4;
	ADCTxData.Chn5 = f_addata5;
    ADCTxData.Chn6 = f_addata6;
    ADCTxData.rtc_tm.tm_year = rtc_tm.tm_year+1900;
	ADCTxData.rtc_tm.tm_mon  = rtc_tm.tm_mon+1;
	ADCTxData.rtc_tm.tm_mday = rtc_tm.tm_mday;
	ADCTxData.rtc_tm.tm_hour = rtc_tm.tm_hour;
	ADCTxData.rtc_tm.tm_min  = rtc_tm.tm_min;
	ADCTxData.rtc_tm.tm_sec  = rtc_tm.tm_sec;
	ADCTxData.crc32 = crc32((const unsigned char *)&ADCTxData, 68);//对前面字节做校验

	//加锁
	pthread_rwlock_wrlock(&g_rwlock);
	if(ppbuf_get_full_signal(p,false)!=true){
		printf("put_index %d \n", p->put_index);
		ppbuf_put_ping(p, &ADCTxData, sizeof(ADCTxData));//把ADCTxData当作unsigned char写进去
		
	}else{
		printf("---------full-------\n");
		ppbuf_get_full_signal(p,true);//满了交换buff

		printf("put_index %d \n", p->put_index);//换完了buff再写数据
		ppbuf_put_ping(p, &ADCTxData, sizeof(ADCTxData));//把ADCTxData当作unsigned char写进去

		int ret = sem_post(&g_sem);//发送信号让tf模块取读数据
		if(ret !=0)
		{
            pthread_rwlock_unlock(&g_rwlock);
			perror("sem_post err");
			exit(errno);
		}
	}
	pthread_rwlock_unlock(&g_rwlock);
	ioctl(wdt_fd, WDIOC_KEEPALIVE, NULL);//喂狗


}

void *sadc_init(void *Pdata)
{
	

	unsigned int vref = 1800;
	char aux_name[32];
	p=(ppbuf_t*)Pdata;
	sprintf(aux_name, "/dev/ingenic_adc_aux_0");
	fd = open (aux_name,O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(errno);
	}

	sprintf(aux_name, "/dev/ingenic_adc_aux_1");
	fd1 = open (aux_name,O_RDWR);
	if (fd1 < 0) {
		perror("open");
		exit(errno);
	}

	sprintf(aux_name, "/dev/ingenic_adc_aux_2");
	fd2 = open (aux_name,O_RDWR);
	if (fd2 < 0) {
		perror("open");
		exit(errno);
	}

	sprintf(aux_name, "/dev/ingenic_adc_aux_3");
	fd3 = open (aux_name,O_RDWR);
	if (fd3 < 0) {
		perror("open");
		exit(errno);
	}

	sprintf(aux_name, "/dev/ingenic_adc_aux_4");
	fd4 = open (aux_name,O_RDWR);
	if (fd4 < 0) {
		perror("open");
		exit(errno);
	}

	sprintf(aux_name, "/dev/ingenic_adc_aux_5");
	fd5 = open (aux_name,O_RDWR);
	if (fd5 < 0) {
		perror("open");
		exit(errno);
	}


	ioctl(fd,ADC_SET_VREF,&vref);
	ioctl(fd1,ADC_SET_VREF,&vref);
	ioctl(fd2,ADC_SET_VREF,&vref);
	ioctl(fd3,ADC_SET_VREF,&vref);
	ioctl(fd4,ADC_SET_VREF,&vref);
	ioctl(fd5,ADC_SET_VREF,&vref);
	
	struct itimerval value;
	value.it_value.tv_sec =1;
	value.it_value.tv_usec=0;
	value.it_interval.tv_sec=2;
	value.it_interval.tv_usec=0;
	
	setitimer(ITIMER_PROF,&value,NULL);

	struct sigaction act;
	act.sa_handler=adc_get_record;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGPROF,&act,NULL);

	//初始化看门狗
	int wdt_ret=-1,ops=-1,timeout=10;
    wdt_fd=open("/dev/watchdog",O_RDWR);

    if(wdt_fd<0){
        perror("open /dev/watchdog fail!!\n");
        exit(errno);
    }
	ops=WDIOS_DISABLECARD;
    wdt_ret=ioctl(wdt_fd, WDIOC_SETOPTIONS, &ops);
    if(wdt_ret<0){
        perror("close wdt fail!!\n");
        exit(errno);
    }

	wdt_ret=ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);
    if(wdt_ret<0){
        perror("WDIOC_SETTIMEOUT fail!!\n");
        exit(errno);
    }

	ops=WDIOS_ENABLECARD;
	wdt_ret=ioctl(wdt_fd, WDIOC_SETOPTIONS, &ops);
    if(wdt_ret<0){
        perror("open wdt fail!!\n");
        exit(errno);
    }

	while(1);

}
