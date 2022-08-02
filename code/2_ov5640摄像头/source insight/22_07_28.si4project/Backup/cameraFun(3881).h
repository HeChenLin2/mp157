#ifndef _CARMRA_FUN_H_
#define _CARMRA_FUN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/videodev2.h>
#include <stdint.h>
#include <asm/types.h>
#include <pthread.h>
#include <poll.h>
#include "drmFun.h"

typedef struct VideoBuffer		/*	用于存放数据帧信息		*/
    {
        unsigned char *pStart;	/*	首地址	*/
        unsigned int uLength;	/*	长度	*/
     }VideoBuffer;

 struct cameraInf{
	int fd;
	int uCount;		//缓冲帧个数
	int	nCount;		//当前读出的缓冲帧下标
	struct v4l2_capability cap;
	struct v4l2_fmtdesc desc;
	struct v4l2_frmsizeenum frmsize;
    struct v4l2_format fmt;
    struct v4l2_buffer buf;				/*	驱动中的一帧数据，保存内核中帧缓存的相关信息的结构体*/
    struct v4l2_requestbuffers rb;
	VideoBuffer * pBuffers;
	pthread_mutex_t mutex;
	
}cameraInf_t;

extern struct cameraInf camera;
int camera_open(char * devname,struct cameraInf * camera);
int v4l2_init(struct cameraInf * camera);
void free_v4l2_memory(struct cameraInf * camera);
int camera_thread(struct cameraInf * camera);
int convert_yuv_to_rgb565_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
int convert_yuv_to_rgb_pixel(int y, int u, int v);
#endif


