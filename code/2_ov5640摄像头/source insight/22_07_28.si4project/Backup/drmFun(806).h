#ifndef _DRM_FUN_H_
#define _DRM_FUN_H_

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

 struct kms_config{
 	 uint32_t	fd;
	 uint32_t fb_widht;	//缓冲区对象宽度
	 uint32_t fb_hight;	//缓冲区对象高度
	 uint32_t fb_line_size;	//缓冲区对象每行像素数，单位字节
	 uint32_t fb_size;	//缓冲区对象总的的像素数，单位字节
	 uint32_t fb_handle;	//缓冲区对象的引用句柄
	 uint32_t*fb_map;	//指向缓冲区对象的内存映射地址
	 
	 uint32_t fb_id;	//扫描缓冲区的缓冲区对象ID
	 uint32_t conn_id;	//与缓冲区绑定的连接器ID
	 uint32_t encoder_id;
	 uint32_t crtc_id;	//与连接器绑定的CRTC ID
	 drmModeCrtc * old_crtc;	//更改前crtc配置，退出时恢复原模式
	 drmModeModeInfo mode;		//使用的显示配置模式	 
 };
 
 extern struct kms_config kms;
 extern int bmpmode;
 extern const char * lcddevname;
 
int drm_Open(int * pfd,const char * devname);
int drm_FindConnector(int fd,drmModeRes *resources);
int drm_FindEncoder(int fd,drmModeRes *resources,drmModeConnector * connector);
int drm_kms_init(drmModeRes *resources,drmModeConnector * connector,drmModeEncoder *encoder);
int drm_find_crtc(int fd,drmModeRes *resources,drmModeConnector * connector);
int drm_crtc_fb(int fd);
int drm_fb_mmap(int fd);
int drm_set_kms(int fd);
int drm_free_fb(int fd);
int drm_init(int fd);
void lcd_show_pic(void * colorData,long  biWidth,long  biHeight,int biBitCount);
void drm_draw_background(void);
void lcd_show_camear(unsigned char * colorData,long  biWidth,long  biHeight);




#endif