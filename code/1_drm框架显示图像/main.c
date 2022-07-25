#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <libdrm/xf86drm.h>
#include <libdrm/xf86drmMode.h>
#include <assert.h>
#include "drmFun.h"
#include "bmp.h"
#include <stdlib.h>
 
 
const char * devname = "/dev/dri/card0";
struct kms_config kms;
int bmpmode = 0;	//0:rgb565 	1:rgb888


 int main(int argc , char * argv[])
 {
	int ret = 0,fd = 0,i = 0;
	void * colorData = NULL;
	long  biWidth, biHeight;
	int biBitCount;
	if(argc != 2){
		printf("usage:<a.out><pic_name>\r\n");
		return -1;
	}
	
	/*打开/dev/dri/card0设备*/
	if(drm_Open(&fd,devname)){
		printf("drm device %s open fail\r\n",devname);
		return -1;
	}
	
	#if 0
		
	/*初始化drm*/
	if(drm_init(fd) < 0){
		printf("drm init fail\r\n");
		close(fd);
		return -1;
	}
	#endif
	
	

	
	if(readData(argv[1],&colorData,&biWidth,  &biHeight ,&biBitCount) == 0){	//获取图像数据成功
	#if 1
		if(biBitCount == 16)
			bmpmode = 0;
		else
			bmpmode = 1;
		
		/*初始化drm*/
		if(drm_init(fd) < 0){
			printf("drm init fail\r\n");
			close(fd);
			return -1;
		}
	#endif
	
	
		//写入图像到Lcd
		lcd_show_pic(colorData,  biWidth,  biHeight, biBitCount);
		free(colorData);
		colorData = NULL;
	}
	
	
	
	
	
	
	
	drm_free_fb(fd);
	close(fd);
	return -1;
 }
 




 