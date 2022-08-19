#include "cameraFun.h"



/************************************************************************
*函数名字：camera_open
*函数功能：打开camear设备节点，并获取摄像头相关信息及支持的图像采集格式
*函数参数：devname：camear设备节点				camera：struct cameraInf结构体指针
*函数返回：0：成功 其他：失败
**************************************************************************/
int camera_open(char * devname,struct cameraInf * camera)
{
	int i = 0;
	int fd = open(devname,O_RDWR);
	if(fd < 0){
		printf("open %s fail\r\n",devname);
		goto err1;
	}

	memset(&camera->cap,0,sizeof(camera->cap));
	if(-1 == ioctl(fd, VIDIOC_QUERYCAP, &camera->cap))	/***填充cap里面的成员***/
  	{
  		perror("ioctrl VIDIOC_QUERYCAP");
    	goto err2;
  	}

	if(! (camera->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
   {
    	goto err2;				/***查询V4L2是否支持图像捕捉功能***/
   }

  if(! (camera->cap.capabilities & V4L2_CAP_STREAMING))
   {
      goto err2;				/***查询V4L2是支持数据流类型***/
   }
  /***打印V4L2设备的相关信息***/
   printf("****************Capability infrmations****************\r\n");
   printf("driver : %s \n", camera->cap.driver);
   printf("card : %s \n", camera->cap.card);
   printf("bus_info : %s \n", camera->cap.bus_info);
   printf("version : %08x \n", camera->cap.version);
   printf("capabilities : %08x\n", camera->cap.capabilities);


   memset(&camera->desc,0,sizeof(camera->desc));
   camera->desc.index = 0;
   camera->desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while(ioctl(fd,VIDIOC_ENUM_FMT,&camera->desc) == 0)
    {
    	i = 0;
		printf("%d.%s\n",camera->desc.index+1, camera->desc.description);
		camera->frmsize.pixel_format = camera->desc.pixelformat;
		camera->frmsize.index = 0;
		while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &camera->frmsize) >= 0){
			if(camera->frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE){
				printf("\t格式%d %dx%d\n",++i, camera->frmsize.discrete.width, camera->frmsize.discrete.height);
			}else if(camera->frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE){
				 printf("\t格式%d %dx%d\n",++i, camera->frmsize.discrete.width, camera->frmsize.discrete.height);
			}
			camera->frmsize.index++;
		}	
		camera->desc.index++;
    }
	printf("**************************************************\r\n");	
	camera->fd = fd;
	
  	return 0;
err2:
	close(fd);
err1:
	return -1;
}



/************************************************************************
*函数名字：v4l2_init
*函数功能：初始化摄像头，包括设置摄像头采集格式，申请缓冲帧，内存映射及缓冲帧入队操作
*函数参数：camera：struct cameraInf结构体指针
*函数返回：0：成功 其他：失败
**************************************************************************/

int v4l2_init(struct cameraInf * camera)
{
	unsigned int numBufs = 0;
	
	
	/***设置视频捕捉的格式，例如设置视频图像数据的长、宽，图像格式(GPEG,YUYV,RGB)***/
	memset(&camera->fmt, 0, sizeof(camera->fmt));
	camera->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			/***视频数据流类型，永远都是这个类型***/
   	camera->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;	/***视频数据的格式：GPEG,YUYV,RGB***/
   	camera->fmt.fmt.pix.width = 640;                         /***视频图像的宽***/
   	camera->fmt.fmt.pix.height = 480;						/***视频图像的长***/
   	camera->fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;		/***设施视频的区域***/

	if(-1 == ioctl(camera->fd, VIDIOC_S_FMT, &camera->fmt))	/***设置视频捕捉格式***/
   {
       perror("ioctl VIDIOC_S_FMT error");
       return	-1;
   }

	ioctl(camera->fd, VIDIOC_G_FMT, &camera->fmt);
   if (camera->fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
   {
       printf("set V4L2_PIX_FMT_RGB565 fail\r\n");
       return	-1;
   }

   /***********向内核申请帧缓存*********/
   camera->uCount = 4;
   memset(&camera->rb, 0, sizeof(camera->rb));
   camera->rb.count = camera->uCount;				/*	缓存数量--->  数据帧的个数	*/
   camera->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	/*	数据帧流类型	*/
   camera->rb.memory = V4L2_MEMORY_MMAP;			/*	内存映射的方式采集数据	*/
	/*	VIDIOC_REQBUFS表示分配内存，调用ioctl使配置生效*/
    if(-1 == ioctl(camera->fd, VIDIOC_REQBUFS, &camera->rb))
    {
        perror("ioctl VIDIOC_REQBUFS error");
        return	-1;
    }

	if((camera->rb.count < 2) || (camera->rb.count > 5))
     {
     	printf("缓冲帧个数太大或太小\r\n");
       return	-1;			/*	保证帧缓存的数量不易太大也不易太小*/
     }

	camera->pBuffers = NULL;
	camera->pBuffers = (VideoBuffer *)calloc(camera->uCount, sizeof(VideoBuffer));
     if(NULL == camera->pBuffers)
      {
         perror("calloc error");
         return	-1;
      }


	 printf("*******************mmap fruit**********************\r\n");
     for(numBufs = 0; numBufs < camera->uCount; numBufs++)	/*	映射所有的帧缓存	*/
     {
         memset(&camera->buf, 0, sizeof(camera->buf));

         camera->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;	/*	数据流类型	*/
         camera->buf.memory = V4L2_MEMORY_MMAP;				/*	内存映射	*/
         camera->buf.index = numBufs;						/*	帧号		*/

         if(-1 == ioctl(camera->fd, VIDIOC_QUERYBUF, &camera->buf))	/*	获取相应的帧缓存信息	*/
         {
              perror("ioctl VIDIOC_QUERYBUF error");
              return	-1;
         }

		  camera->pBuffers[numBufs].pStart = NULL;
          camera->pBuffers[numBufs].uLength = camera->buf.length;		/*	用户空间的长度	*/
          camera->pBuffers[numBufs].pStart = (unsigned char *)mmap(NULL, camera->buf.length, PROT_READ |PROT_WRITE, MAP_SHARED, camera->fd, camera->buf.m.offset);	/*	映射内存	*/
          if(NULL == camera->pBuffers[numBufs].pStart)
          {
             perror("mmap error");
             return	-1;
          }
		  
		  printf("buff%d	start:%#x	length:%d\r\n",numBufs+1,camera->pBuffers[numBufs].pStart,camera->pBuffers[numBufs].uLength);
		/*	将申请到的帧数据入队列，存储采集到的数据	*/
	       if(-1 == ioctl(camera->fd, VIDIOC_QBUF, &camera->buf))
	       {
	          perror("ioctl VIDIOC_QBUF error");
	          return	-1;
	        }

	 }
   	printf("**************************************************\r\n");

	if(camera->fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV){		//yuyv的数据格式
		int n = 2;
		if(bmpmode)
			n = 3;

		/*申请一些内存空间*/
		camera->tid = NULL;
		camera->tid = (pthread_t *)malloc(sizeof(pthread_t) * (camera->uCount + 1)  );	//每个缓冲帧都有一个线程来处理
		camera->sem = NULL;
		camera->sem = (sem_t *)malloc(sizeof(sem_t) * (camera->uCount)  );
		
		for(int i = 0;i < camera->uCount;i++){
			camera->rgbdata[i] = NULL;
			camera->rgbdata[i] = (unsigned char *)malloc(sizeof(unsigned char) * camera->fmt.fmt.pix.width * camera->fmt.fmt.pix.height * n );
			if(camera->rgbdata[i] == NULL){
				printf("rgbdata[%d] malloc rgbdata fail\r\n",i);
				return -1;
			}
			sem_init(&camera->sem[i],0,0);
		}
		camera->msg_id = createMsgQueue();
		
		#if 0
		camera->rgbdata1 = NULL;
		camera->rgbdata1 = (unsigned char *)malloc(sizeof(unsigned char) * camera->fmt.fmt.pix.width * camera->fmt.fmt.pix.height * n );
		if(camera->rgbdata1 == NULL){
			printf("malloc rgbdata fail\r\n");
			return -1;
		}


		camera->rgbdata2 = NULL;
		camera->rgbdata2 = (unsigned char *)malloc(sizeof(unsigned char) * camera->fmt.fmt.pix.width * camera->fmt.fmt.pix.height * n );
		if(camera->rgbdata2 == NULL){
			printf("malloc rgbdata fail\r\n");
			return -1;
		}
		#endif
		
		yuv_rgb_table_init();
	}
	

   return 0;

}


/************************************************************************
*函数名字：free_v4l2_memory
*函数功能：解除内存映射，释放内存
*函数参数：camera：struct cameraInf结构体指针
*函数返回：无
**************************************************************************/

void free_v4l2_memory(struct cameraInf * camera)
{
	unsigned int numBufs = 0;
	int i = 0;
	for(numBufs = 0; numBufs < camera->uCount; numBufs++)	/*	映射所有的帧缓存	*/
	{
		if(camera->pBuffers[numBufs].pStart != NULL)
			munmap(camera->pBuffers[numBufs].pStart, camera->pBuffers[numBufs].uLength);//解除隐射才能释放cache空间
	}

	if(camera->tid != NULL)
		free(camera->tid);

	if(camera->sem != NULL)
		free(camera->sem);

	for( i = 0;i < camera->uCount;i++)
		if(camera->rgbdata[i] != NULL)
			free(camera->rgbdata[i]);
}


void timec(void)
{
    time_t timep;
    struct tm *tmp;
 
    time(&timep);
    tmp = localtime(&timep);
 
    printf("%d-%d-%d %d:%d:%d\n",1900 + tmp->tm_year,1 + tmp->tm_mon, tmp->tm_mday,
            tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
 
}

#if 0
/************************************************************************
*函数名字：thread_fun
*函数功能：线程函数，循环获取摄像头数据
*函数参数：arg：struct cameraInf结构体指针
*函数返回：无意义
**************************************************************************/
static void * thread_fun(void * arg)
{
	int ret = 0;
	int who = 0;
	struct v4l2_buffer tV4l2Buf;
	struct cameraInf * camera = (struct cameraInf *)arg;
	struct pollfd fd[1] = {
		[0] = {
			.fd = camera->fd,
			.events = POLLIN,
			}
	};
	unsigned char * rgbdata;
	tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;

	//双线程处理数据,开启第二个线程
	if(camera->tid[0] == pthread_self()) {
		if(pthread_create(&camera->tid[1], NULL,thread_fun, (void *)camera) != 0){
			printf("camera pthread_create fail\r\n");
		}
		 rgbdata = camera->rgbdata1;	//两个线程不同的rgb数据缓冲区
		who = 1;
	}else{
		rgbdata = camera->rgbdata2;
		who = 2;
	}	

	while(1){
		poll(fd, 1, -1);
		if(fd[0].revents & POLLIN){		//有摄像头数据

			//出队
			ret = ioctl(camera->fd, VIDIOC_DQBUF, &tV4l2Buf);
			if(ret < 0){
				printf("Unable to dequeue buffer.\n");
				return NULL;
			}

			switch(camera->fmt.fmt.pix.pixelformat)
			{
				case V4L2_PIX_FMT_YUYV:
					convert_yuv_to_rgb565_buffer(camera->pBuffers[tV4l2Buf.index].pStart, rgbdata, camera->fmt.fmt.pix.width,camera->fmt.fmt.pix.height);
					sendMsgQueue( camera->msg_id , 1234, who);	//写入消息队列
					//sem_post(&camera->sem);		//释放信号量	
					break;
			
			}

			
			
			//入队
			ret = ioctl(camera->fd, VIDIOC_QBUF, &tV4l2Buf);
		    if (ret < 0)
		    {
		         printf("Unable to queue buffer.\n");
		         return NULL;
		    }
			
		}
	}
	return NULL;
}
#endif



static void * ProcessingImages (void * arg)
{
	int who = 0,ret = 0;
	struct v4l2_buffer tV4l2Buf;
	struct cameraInf * camera = (struct cameraInf *)arg;

	
	for(int i = 1; i < camera->uCount + 1;i++ )		//确定自己是第几个处理图像的线程
		if(camera->tid[i] == pthread_self()){
			who = i;
			break;
		}

	tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	tV4l2Buf.index = who - 1;
	while(1)
	{
		
		sem_wait(&camera->sem[who-1]);
		switch(camera->fmt.fmt.pix.pixelformat)
		{
			case V4L2_PIX_FMT_YUYV:
				convert_yuv_to_rgb565_buffer(camera->pBuffers[tV4l2Buf.index].pStart, camera->rgbdata[who-1], camera->fmt.fmt.pix.width,camera->fmt.fmt.pix.height);
				sendMsgQueue( camera->msg_id , 1234, who-1);	//写入消息队列,传递给写入lcd的线程
				break;
			
		}
		//printf("--ProcessingImages =%d--\r\n",tV4l2Buf.index);
	
	  	//入队
	 	ret = ioctl(camera->fd, VIDIOC_QBUF, &tV4l2Buf);
		if (ret < 0)
		{
			 printf("Unable to queue buffer.\n");
			 return NULL;
		}

	}
	return NULL;
}



/************************************************************************
*函数名字：thread_fun
*函数功能：线程函数，循环获取摄像头数据
*函数参数：arg：struct cameraInf结构体指针
*函数返回：无意义
**************************************************************************/
static void * thread_fun(void * arg)
{
	int ret;
	struct v4l2_buffer tV4l2Buf;
	struct cameraInf * camera = (struct cameraInf *)arg;
	struct pollfd fd[1] = {
		[0] = {
			.fd = camera->fd,
			.events = POLLIN,
		}
	};

	tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;

	for(int i = 1;i < camera->uCount + 1;i++)
		if(pthread_create(&camera->tid[i], NULL,ProcessingImages, (void *)camera) != 0){
			printf("camera->tid[i] camera pthread_create fail\r\n",i);
			return NULL;
		}

	while(1){
		poll(fd, 1, -1);
		if(fd[0].revents & POLLIN){		//有摄像头数据

			//出队
			ret = ioctl(camera->fd, VIDIOC_DQBUF, &tV4l2Buf);
			if(ret < 0){
				printf("Unable to dequeue buffer.\n");
				return NULL;
			}
			sem_post( &camera->sem[tV4l2Buf.index] );
			
		}
	}
	return NULL;
}



/************************************************************************
*函数名字：camera_thread
*函数功能：启动摄像头采集，开启线程获取摄像头数据
*函数参数：camera：struct cameraInf结构体指针
*函数返回：0：成功  			其他：失败
**************************************************************************/
int camera_thread(struct cameraInf * camera)
{
	int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iError;
	int * p;

     iError = ioctl(camera->fd, VIDIOC_STREAMON, &iType);
     if (iError)     {
         printf("Unable to start camear.\n");
         return -1;
    }
	 
	if(pthread_create(&camera->tid[0], NULL,thread_fun, (void *)camera) != 0){
		printf("camera->tid[0] camera pthread_create fail\r\n");
		return -1;
	}
	return 0;
}










