#ifndef _CONVERSION_PICDATE_H_
#define _CONVERSION_PICDATE_H_

#include <stdio.h>


int convert_yuv_to_rgb565_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
int convert_yuv_to_rgb_pixel(int y, int u, int v);
void yuv_rgb_table_init(void);
int convert_yuv_to_rgb_pixel_table(int y, int u, int v);



#endif

