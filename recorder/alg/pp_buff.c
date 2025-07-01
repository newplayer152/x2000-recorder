#include "pp_buff.h"
#include <string.h>


int ppbuf_put_ping(ppbuf_t *p, void *data, int size){//只管往ping里面写
	if(p==NULL||data==NULL||size<=0){//合法性判断
		perror("put fail err\n");
		return -1;
	}
	if(p->put_index+size>p->buffer_size){//能否写的下
		perror("put size overflow  fail err\n");
	}
	memcpy(&(p->buffer_data)[p->ping *p->buffer_size + p->put_index],data,size);//写数据
	p->put_index += size;
	p->full_signal = p->put_index==p->buffer_size ? true : false;//判满
	return 0;
}
int ppbuf_get_pong(ppbuf_t *p, void *data, int size){//只管从pong里面读
	if(p==NULL||data==NULL||size<=0){//合法性判断
		perror("get fail err\n");
		return -1;
	}  
	if(p->get_index+size>p->buffer_size){//能否读的了
		perror("get size overflow  fail err\n");
	}
	memcpy(data,&(p->buffer_data)[p->pong *p->buffer_size + p->get_index],size);//读数据
	p->get_index += size;
	return 0;                                                                                                                                           
}
unsigned char *ppbuf_get_addr(ppbuf_t* p, int size){
	if(p==NULL||size<=0){
		perror(" fail err\n");
	} 
	return (unsigned char *)&(p->buffer_data[p->pong * p->buffer_size]) ;
}

bool ppbuf_get_full_signal(ppbuf_t *p, bool consume){//判断是否写满 以及通过consume选择是否交换ping pong
	bool ret = p->full_signal;
	if(p==NULL){
		perror("signal deal fail err\n");
		return false;
	}
	if(consume&&ret){
		p->full_signal=false;
		p->ping=p->ping ^ p->pong;
		p->pong=p->ping ^ p->pong;
		p->ping=p->ping ^ p->pong;
		p->get_index=0;
		p->put_index=0;
	}
	return ret;
}