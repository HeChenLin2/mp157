#include "drmFun.h"

 int drm_init(int fd)
 {
	 drmModeRes *resources = NULL;
	 /*获取drmModeRes资源*/
	resources = drmModeGetResources(fd);
	if(NULL == resources){
		printf("drm Get Resources fail\r\n",lcddevname);
		goto err;
	}
	
	/*查找并绑定crtc,encoder,connector*/
	if(drm_FindConnector(fd,resources) == -1){
		printf("没有查找到与合适的连接器、编码器、crtc\r\n");
		goto err1;
	}
	
	/*为crtc申请扫描缓冲区*/
	if(drm_crtc_fb(fd) < 0){
		printf("为crtc申请扫描缓冲区失败\r\n");
		goto err1;
	}
		
	if(drm_set_kms(fd) == -1){
		goto err2;
	}
	return 0;
	err2:
		drm_free_fb(fd);
	err1:
		drmModeFreeResources(resources);
	err:
		return -1;
	 
 }
 
 
  int drm_Open(int * pfd,const char * devname)
 {
	int fd = 0,ret = 0;
	uint64_t value = 0;
	fd = open(devname,O_RDWR);
	if(fd < 0){
		printf("cannot open drm device:%s\r\n",devname);
		return fd;
	}
	 /*检查drm设备是否支持DRM_CAP_DUMB_BUFFER特性*/
	 ret = drmGetCap(fd,DRM_CAP_DUMB_BUFFER,&value);
	 if(ret < 0 || !value){
		 printf("%s cannot support DRM_CAP_DUMB_BUFFER\r\n",devname);
		 close(fd);
		 return -1;
	 }
	 *pfd = fd;
	 return 0;
 }
 
 
 
 /*查找资源当中的连接器*/
 int drm_FindConnector(int fd,drmModeRes *resources)
 {
	 int i= 0;
	 drmModeConnector * connector = NULL;
	 printf("资源当中的连接器数量为:%d\r\n",resources->count_connectors);
	 //逐一从连接器中查找编码器
	 for(i = 0; i < resources->count_connectors;i++)		
	 {
		 printf("******************查找连接器%dstart******************************************\r\n",i);
		 /*获取连接器*/
		connector = drmModeGetConnector(fd, resources->connectors[i]);
		if(connector == NULL){
			printf("获取连接器%d失败\r\n",i);
			continue;
		}
		printf("获取连接器%d成功,连接器Id为%d\r\n",i,connector->connector_id);
		 /*判断连接器是否已经连接，否则忽略*/
		 /*判断连接器是否有 有效的模式，否则忽略*/
		if(connector->connection != DRM_MODE_CONNECTED || connector->modes == 0 ){
			printf("id为:%d的连接器没有连接、或者无效模式\r\n",connector->connector_id);
			continue;
		}
			
		/*查找编码器*/
		if(drm_FindEncoder(fd ,resources,connector) < 0)	//没有找到合适的编码器
		{
			drmModeFreeConnector(connector);//释放连接器
			continue;
		}
		
		drmModeFreeConnector(connector);//释放连接器
		
		return 0;
	 }
	 return -1;
 }
 
	/*查找编码器*/
  int drm_FindEncoder(int fd,drmModeRes *resources,drmModeConnector * connector)
  {
	drmModeEncoder *encoder = NULL;
	/*判断连接器是否有绑定的编码器*/
	if(connector->encoder_id){
		encoder = drmModeGetEncoder(fd, connector->encoder_id);
		printf("连接器Id为%d的连接器绑定了编码器,该编码器的id为:%d\r\n",connector->connector_id,encoder->encoder_id);
	}
	else {
		encoder = NULL;
		printf("连接器Id为%d的连接器没有绑定编码器\r\n",connector->connector_id);
	}
	
	/*判断该绑定的编码器是否绑定了crtc*/
	if(encoder != NULL){
		if(encoder->crtc_id){
			printf("编码器 id:%d 绑定了ctrc id:%d\r\n",encoder->encoder_id,encoder->crtc_id);
			drm_kms_init(resources,connector,encoder);
			drmModeFreeEncoder(encoder);//释放编码器
			return 0;
		}
		drmModeFreeEncoder(encoder);//释放编码器
		encoder = NULL;
	}else
		printf("编码器 id:%d 没有绑定ctrc\r\n",encoder->encoder_id);
		
	//如果该连接器没有绑定编码器，或者编码器没有绑定crtc，则查找与编码器适配的crtc
	return drm_find_crtc(fd, resources, connector); 	
  }
  
  int drm_kms_init(drmModeRes *resources,drmModeConnector * connector,drmModeEncoder *encoder)
  {
	kms.conn_id = connector->connector_id;
	kms.encoder_id = encoder->encoder_id;
	kms.crtc_id = encoder->crtc_id;
	memcpy(&kms.mode,&connector->modes[0],sizeof(kms.mode));
	kms.fb_widht = kms.mode.hdisplay;
	kms.fb_hight = kms.mode.vdisplay;
	printf("bind crtc %d + encoder %d + connector %d success\r\n",kms.crtc_id,kms.encoder_id,kms.conn_id); 
	return 0;
  }
  
  
  int drm_find_crtc(int fd,drmModeRes *resources,drmModeConnector * connector)
  {
	  drmModeEncoder *encoder = NULL;
	  int i,j;
	  
	  for(i = 0; i < connector->count_encoders;i++)		//查找所有的编码器
	  {
		  encoder = drmModeGetEncoder(fd, connector->encoder_id);
		  if(!encoder)
			continue;
		  for(j = 0 ; j < resources->count_crtcs;j++)	//逐一给编码器查找适配的crtc
		  {
			  /*判断crtc与编码器是否适配*/
			  if( !(encoder->possible_crtcs & (1 << j)))
				continue;
			  drm_kms_init(resources,connector,encoder);
			  drmModeFreeEncoder(encoder);//释放编码器
			  return 0;
		  }
		   drmModeFreeEncoder(encoder);//释放编码器
		   encoder = NULL;  
	  }
	  printf("没有查找到与编码器适配的crtc\r\n");
	  return -1;
  }
  
  
  int drm_crtc_fb(int fd)
  {
	int rval;
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	creq.height = kms.fb_hight;
	creq.width = kms.fb_widht;
	if(bmpmode)
		creq.bpp = 32;
	else
		creq.bpp = 16;
		
	//创建dumb扫描输出缓冲区
	rval = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (rval < 0) {
		printf("%s-->create dumb buffer error:%m\n", __func__);
		return rval;
	}
	kms.fb_line_size = creq.pitch;
	kms.fb_handle = creq.handle;
	kms.fb_size = creq.size;
	
	//使用dumb缓冲区对象创建帧缓冲区
	if(bmpmode)
		rval = drmModeAddFB(fd,kms.fb_widht,kms.fb_hight,24,32,kms.fb_line_size,kms.fb_handle,&kms.fb_id);
	else
		rval = drmModeAddFB(fd,kms.fb_widht,kms.fb_hight,16,16,kms.fb_line_size,kms.fb_handle,&kms.fb_id);
	if(rval < 0){
		printf("%s-->create framebuffer error: %m\n", __func__);
		
		goto err;
	}
	
	//内存映射
	if(drm_fb_mmap(fd) < 0){
		goto err1;
	}
	
	/*kms模式设置*/
	drm_set_kms(fd);
	return 0;
	
	
err1:
	drmModeRmFB(fd,kms.fb_id);
err:
	memset(&dreq,0,sizeof(dreq));
	dreq.handle = creq.handle;
	drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
	return -1;
  }
  
  int drm_fb_mmap(int fd)
  {
	int rval;
	struct drm_mode_map_dumb mreq;
	memset(&mreq,0,sizeof(mreq));
	mreq.handle = kms.fb_handle;
	/* 准备内存映射:获取dumb缓冲区的mmap偏移量 */
	rval = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (rval) {
		printf("%s-->MODE_MAP_DUMBr error:%m\n", __func__);
		return -errno;
	}
	
	/* 内存映射 */
	kms.fb_map = mmap(0, kms.fb_size, PROT_READ | PROT_WRITE,
										MAP_SHARED, fd, mreq.offset);
										
	if (MAP_FAILED == kms.fb_map) {
		printf("%s-->cannot mmap dumb buffer error:%m\n", __func__);
		return -errno;
	}
	
	memset(kms.fb_map, 0, kms.fb_size);
	return 0; 
  }
  
  int drm_set_kms(int fd)
  {
	kms.old_crtc = drmModeGetCrtc(fd,kms.crtc_id);
	if(drmModeSetCrtc(fd,kms.crtc_id,kms.fb_id,0,0,&kms.conn_id,1,&kms.mode) < 0)
	{
		printf("drmModeSetCrtc fail\r\n");
		return -1;
	}
	return 0;
  }
  
  
  int drm_free_fb(int fd)
  {
	struct drm_mode_destroy_dumb dreq;
	munmap(kms.fb_map,kms.fb_size);
	drmModeRmFB(fd,kms.fb_id);
	memset(&dreq,0,sizeof(dreq));
	dreq.handle = kms.fb_handle;
	drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
	  
}


void lcd_show_camear(unsigned char * colorData,long  biWidth,long  biHeight)
{
	int i = 0,j = 0,z = 0,n = 2;
	uint16_t * p = (uint16_t *) colorData;
	for(i = 0;i < kms.fb_hight;i++){
		z = kms.fb_line_size / n * i;
		for(j = 0; j < kms.fb_widht;j++){
			if((j < biWidth) && (i < biHeight)){
				((uint16_t *)kms.fb_map)[z++] =p[biWidth * i + j];
			}			
		}		
	}
}


void lcd_show_pic(void * colorData,long  biWidth,long  biHeight,int biBitCount)
{
	int i = 0,j = 0,z = 0,n = 2;
	if(biBitCount == 24)
		n = 4;
		
		
	for(i = 0;i < kms.fb_hight;i++){
		z = kms.fb_line_size / n * i;
		for(j = 0; j < kms.fb_widht;j++){
			if((j < biWidth) && (i < biHeight)){
				if(biBitCount == 16)
					((uint16_t *)kms.fb_map)[z++] = *((uint16_t *)colorData+(biWidth * i + j));
				else if(biBitCount == 24)
				((uint32_t *)kms.fb_map)[z++] = *((uint32_t *)colorData+(biWidth * i + j));
			}else{
				if(biBitCount == 16)
					((uint16_t *)kms.fb_map)[z++] = 0xffff;
				else if(biBitCount == 24)
					((uint32_t *)kms.fb_map)[z++] = 0xffffff;
			}
			
		}
		
	}
}



void drm_draw_background(void)
{
	int i,k,n = 2;
	int color[] = {0xf800,0x7e0,0x1f,0xffff};
	printf("宽度:%d,高度:%d\r\n",kms.fb_widht,kms.fb_hight);
	printf("一行的宽度:%d\r\n",kms.fb_line_size);
	printf("缓冲区总的大小:%d\r\n",kms.fb_size);
	if(bmpmode)
		n = 4;
	for(k = 0;k < 4; k++){
		for(i = 0; i < (kms.fb_size / n);i++)
			((uint16_t *)kms.fb_map)[i] = color[k];
		sleep(2);
	}
}
 


#if 0
/*
			typedef struct _drmModeRes {

			int count_fbs;
			uint32_t *fbs;

			int count_crtcs;
			uint32_t *crtcs;

			int count_connectors;
			uint32_t *connectors;

			int count_encoders;
			uint32_t *encoders;

			uint32_t min_width, max_width;
			uint32_t min_height, max_height;
		} drmModeRes, *drmModeResPtr;
	*/
	
	
	/*
			typedef struct _drmModeConnector {
			uint32_t connector_id;
			uint32_t encoder_id; //< Encoder currently connected to 
			uint32_t connector_type;
			uint32_t connector_type_id;
			drmModeConnection connection;
			uint32_t mmWidth, mmHeight; //< HxW in millimeters 
			drmModeSubPixel subpixel;

			int count_modes;
			drmModeModeInfoPtr modes;

			int count_props;
			uint32_t *props; //< List of property ids 
			uint64_t *prop_values; //< List of property values 

			int count_encoders;
			uint32_t *encoders; //< List of encoder ids 
		} drmModeConnector, *drmModeConnectorPtr;
	*/
	
	
	/*
		typedef struct _drmModeEncoder {
			uint32_t encoder_id;
			uint32_t encoder_type;
			uint32_t crtc_id;
			uint32_t possible_crtcs;
			uint32_t possible_clones;
		} drmModeEncoder, *drmModeEncoderPtr;
		*/
		
		/* create a dumb scanout buffer */
		/*
		struct drm_mode_create_dumb {
			__u32 height;
			__u32 width;
			__u32 bpp;
			__u32 flags;
			// handle, pitch, size will be returned 
			__u32 handle;
			__u32 pitch;
			__u64 size;
		};
		*/
		#endif