#include "bmp.h"
#include "stdio.h"
#include <stdlib.h>

void showBmpHead(BITMAPFILEHEADER pBmpHead) 
{  //定义显示信息的函数，传入文件头结构体
    printf("BMP文件大小：%dkb\n", pBmpHead.bfSize/1024);
    printf("保留字必须为0：%d\n",  pBmpHead.bfReserved1);
    printf("保留字必须为0：%d\n",  pBmpHead.bfReserved2);
    printf("实际位图数据的偏移字节数: %d\n",  pBmpHead.bfOffBits);
}

void showBmpInfoHead(BITMAPINFOHEADER pBmpinfoHead) 
{//定义显示信息的函数，传入的是信息头结构体
    printf("位图信息头:\n" );   
    printf("信息头的大小:%d\n" ,pBmpinfoHead.biSize);   
    printf("位图宽度:%ld\n" ,pBmpinfoHead.biWidth);   
    printf("位图高度:%ld\n" ,pBmpinfoHead.biHeight);   
    printf("图像的位面数(位面数是调色板的数量,默认为1个调色板):%d\n" ,pBmpinfoHead.biPlanes);   
    printf("每个像素的位数:%d\n" ,pBmpinfoHead.biBitCount);   
    printf("压缩方式:%d\n" ,pBmpinfoHead.biCompression);   
    printf("图像的大小:%d\n" ,pBmpinfoHead.biSizeImage);   
    printf("水平方向分辨率:%ld\n" ,pBmpinfoHead.biXPelsPerMeter);   
    printf("垂直方向分辨率:%ld\n" ,pBmpinfoHead.biYPelsPerMeter);   
    printf("使用的颜色数:%d\n" ,pBmpinfoHead.biClrUsed);   
    printf("重要颜色数:%d\n" ,pBmpinfoHead.biClrImportant);   
}

void readBmpData_RGB565(FILE * f1,int offBits,u_int16_t * colorData,int count)
{
    int i = 0;
    u_int8_t color[2];
    fseek(f1, offBits, SEEK_SET);
    for(i = 0;i < count;i++)
    {
        fread(color,sizeof(u_int8_t),2,f1);
        *colorData++ = (u_int16_t)color[1] << 8 |  color[0];
    }	
}

void readBmpData_RGB888(FILE * f1,int offBits,u_int32_t * colorData,int count)
{
    int i = 0;
    u_int8_t color[3];
    fseek(f1, offBits, SEEK_SET);
    for(i = 0;i < count;i++)
    {
        fread(color,sizeof(u_int8_t),3,f1);
        *colorData++ = (u_int32_t)color[2] << 16 |  (u_int8_t)color[1] << 8 | (u_int8_t)color[0];
    }	
}


int readData(char * filename,void ** colorData,long * biWidth,long * biHeight,int* biBitCount)
{
	FILE* fp;  
	unsigned short  fileType;  
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	
	fp = fopen(filename, "rb");//读取同目录下的image.bmp文件。
	if(fp == NULL)
    {
        printf("打开%s失败！\n",filename);
        return -1;
    }
	
    fread(&fileType,1,sizeof (unsigned short), fp);  
    if (fileType = 0x4d42)   
    {   
        printf("文件类型标识正确!" );  
        printf("\n文件标识符：%d\n", fileType); 
        fread(&fileHeader, 1, sizeof(BITMAPFILEHEADER), fp);
        showBmpHead(fileHeader);
        fread(&infoHeader, 1, sizeof(BITMAPINFOHEADER), fp);
        showBmpInfoHead(infoHeader);   
		*biWidth = infoHeader.biWidth;
		*biHeight = infoHeader.biHeight;
    }else{
		printf("%s不是一个BMP图片\r\n",filename);
		fclose(fp);   
		return -1;
	}
	*biBitCount = infoHeader.biBitCount;
	if(infoHeader.biBitCount != 16 && infoHeader.biBitCount != 24){
		printf("%s位宽不为16或者24\r\n",filename);
		fclose(fp);   
		return -1;
	}
	if(infoHeader.biBitCount == 16)
		*(u_int16_t**)colorData = (u_int16_t *)malloc(infoHeader.biWidth * infoHeader.biHeight*sizeof(u_int16_t));
	else
		*(u_int32_t**)colorData = (u_int32_t *)malloc(infoHeader.biWidth * infoHeader.biHeight*sizeof(u_int32_t));
		
	if(*colorData == NULL){
		printf("malloc fail\r\n");
		return -1;
	}
	if(infoHeader.biBitCount == 16)
		readBmpData_RGB565(fp,fileHeader.bfOffBits,*(u_int16_t**)colorData,infoHeader.biWidth * infoHeader.biHeight);
	else 
		readBmpData_RGB888(fp,fileHeader.bfOffBits,*(u_int32_t**)colorData,infoHeader.biWidth * infoHeader.biHeight);
	fclose(fp);  
	return 0;
}
