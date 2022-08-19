#include "conversionPicDate.h"



/*一般来说浮点运算比较慢，因此yuyv转rgb时将浮点转化为整数进行计算，计算出结果后再转化回来*/
static const int csY_coeff_16 = 1.164383*(1<<16);
static const int csU_blue_16  = 2.017232*(1<<16);
static const int csU_green_16 = (-0.391762)*(1<<16);  
static const int csV_green_16 = (-0.812968)*(1<<16);
static const int csV_red_16   = 1.596027*(1<<16);


/*采用查表法进一步提升转化速度*/
/*实验测得才mp157开发板上使用查表法并不能对转化速度进行太大优化，提升不大*/
/*因此在本工程中采用处理摄像头数据时开启多线程的方法来提高转化速率，效果明显*/
int y_table[256];
int v_r_table[256];
int v_g_table[256];
int u_g_table[256];
int u_b_table[256];


/*初始化查表法所使用的表格数据*/
void yuv_rgb_table_init(void)
{
	for(int i=0;i<=255;i++)
    {
        y_table[i]=(i-16)*csY_coeff_16;
        v_r_table[i]=(i-128)*csV_red_16;
        v_g_table[i]=(i-128)*csV_green_16;
        u_g_table[i]=(i-128)*csU_green_16;
        u_b_table[i]=(i-128)*csU_blue_16;
    }
}


/*使用直接计算的方法将一组yuv数据转化为一组rgb数据*/
int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
    unsigned	int		pixel32 = 0;
    unsigned	char 	*pixel = (unsigned char *)&pixel32;
    int 				r, g, b;

	
	b= ( csY_coeff_16 * (y - 16) + csU_blue_16*(u - 128) ) >> 16;
	g= ( csY_coeff_16 * (y - 16) + csU_green_16 *(u - 128) + csV_green_16 *(v - 128) ) >> 16;
	r=( csY_coeff_16 * (y - 16) + csV_red_16 *(v - 128) ) >> 16;
   
    pixel[0]  = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    pixel[1]  = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    pixel[2]  = (b > 255) ? 255 : ((b < 0) ? 0 : b);
	
    return pixel32;
}

/*使用查表的方法将一组yuv数据转化为一组rgb数据*/
int convert_yuv_to_rgb_pixel_table(int y, int u, int v)
{
    unsigned	int		pixel32 = 0;
    unsigned	char 	*pixel = (unsigned char *)&pixel32;
    int 				r, g, b;

	
	b= ( y_table[y] + u_b_table[u] ) >> 16;
	g= ( y_table[y] + u_g_table[u] + v_g_table[v] ) >> 16;
	r= ( y_table[y] + v_r_table[v] ) >> 16;
   
    pixel[0]  = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    pixel[1]  = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    pixel[2]  = (b > 255) ? 255 : ((b < 0) ? 0 : b);
	
    return pixel32;
}



/*使用查表法将一帧摄像头数据从Yuyv格式转化为rgb565格式*/
int convert_yuv_to_rgb565_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in = 0, out = 0;
    int y0, u, y1, v;
    unsigned int pixel24;
    unsigned char *pixel = (unsigned char *)&pixel24;
    unsigned int size = width * height * 2;

    for(; in < size; in += 4, out += 4)
     {
          y0 = yuv[in+0];
          u  = yuv[in+1];
          y1 = yuv[in+2];
          v  = yuv[in+3];

			/*转化方法可更改次函数convert_yuv_to_rgb_pixel_table*/
          pixel24 = convert_yuv_to_rgb_pixel_table(y0, u, v);
          rgb[out+1] = (pixel[0] & ~(0x7))  | (pixel[1] >> 5);
          rgb[out+0] = ((pixel[1] << 3) & ~(0x1f)) | (pixel[2] >> 3) ;

          pixel24 = convert_yuv_to_rgb_pixel_table(y1, u, v);
          rgb[out+3] = (pixel[0] & ~(0x7))  | (pixel[1] >> 5);
          rgb[out+2] = ((pixel[1] << 3) & ~(0x1f)) | (pixel[2] >> 3) ;
     }
     return 0;
}







