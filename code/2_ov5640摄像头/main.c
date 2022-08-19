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
#include <semaphore.h>
#include "cameraFun.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msgbufFun.h"

 
 
const char * lcddevname = "/dev/dri/card0";
struct kms_config kms;
struct cameraInf camera;
int bmpmode = 0;	//0:rgb565 	1:rgb888


 int main(int argc , char * argv[])
 {
	int ret = 0,i = 0;
	if(argc != 2){
		printf("usage:<a.out><dev_name>\r\n");
		return -1;
	}

	/*************************摄像头相关初始化********************************/
	if(camera_open(argv[1], &camera)){
		printf("camera_open fail \r\n");
		goto err1;
	}

	if(v4l2_init(&camera)){
		printf("v4l2_init fail\r\n");
		goto err2;
	}

	if(camera_thread(&camera)){
		printf("camera_thread fail\r\n");
		goto err2;
	}
	/**************************************************************************/


	/*************************lcd屏相关初始化**********************************/
	if(drm_Open(&kms.fd,lcddevname)){
		printf("drm_Open faill\r\n");
		goto err2;
	}

	if(drm_init(kms.fd)){
		printf("drm_init fail\r\n");
		goto err3;
	}

	if(drm_thread()){
		printf("drm_thread fail\r\n");
		goto err3;
	}
	
	/**************************************************************************/
	pthread_join(camera.tid[0],NULL);
	pthread_join(camera.tid[1],NULL);
	pthread_join(kms.tid,NULL);

err3:
	close(kms.fd);
 err2:
 	free_v4l2_memory(&camera);
	close(camera.fd);
 err1:
	return -1;
 }
 




 
