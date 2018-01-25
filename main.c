#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "qdbmp.h"


#define FILE_NAME "mario.bmp"

void print_image(BMP* bmp, int x, int y) {
	int i, j, k;
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			unsigned char r, g, b;
			BMP_GetPixelRGB(bmp, j, i, &r, &g, &b);
			printf("x:%d, y:%d, r:%d, g:%d, b:%d\n", j, i, r, g, b);
		}
	}
}

void resize_nearest_neighbour(unsigned char *original_data, int x,
		int y, int n, int factor, unsigned char *result_data) {
	
	int i, j, k;
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			for (k = 0; k < factor; k++) {
				// powielenie pixeli w wierszu
//				memcpy(result_data + (factor*i*x*n*factor) + (factor*j*n) + (k*n),
//						original_data + (i*x*n) + (j*n), n);
			}
		}
		for (k = 0; k < factor; k++) {
			// powielenie wierszy
//			memcpy(result_data + (factor*i*x*n*factor) + (x*n*factor*k),
//					result_data + (factor*i*x*n*factor), x*n*factor);
		}
	}
}

unsigned char* get_pixel(BMP* bmp, int x, int y,
		unsigned char* r, unsigned char* g, unsigned char* b) {

	int width = BMP_GetWidth(bmp);
	int height = BMP_GetHeight(bmp);
	
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	
	BMP_GetPixelRGB(bmp, x, y, r, g, b);
	//printf("get_pixel x:%d, y:%d, r:%d, g:%d, b:%d\n", x, y, *r, *g, *b);
}

float cubic_hermite(float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;
 
    return a*t*t*t + b*t*t + c*t + d;
}

void bicubic_sample(BMP* original_bitmap, float u, float v,
		unsigned char* r, unsigned char* g, unsigned char* b) {
	
	int width = BMP_GetWidth(original_bitmap);
	int height = BMP_GetHeight(original_bitmap);
	
	float x = (u * width) - 0.5; if (x<0) x = 0;
	int xint = (int)x;
	float xfract = x - floor(x);
	
	float y = (v * height) - 0.5; if (y<0) y = 0;
	int yint = (int)y;
	float yfract = y - floor(y);
	
	//printf("x: %f, xint: %d, y: %f, yint: %d\n", x, xint, y, yint);

	unsigned char p00[3]; 
	unsigned char p10[3];
	unsigned char p20[3];
	unsigned char p30[3];
	
	unsigned char p01[3];
	unsigned char p11[3];
	unsigned char p21[3];
	unsigned char p31[3];
	
	unsigned char p02[3];
	unsigned char p12[3];
	unsigned char p22[3];
	unsigned char p32[3];
	
	unsigned char p03[3];
	unsigned char p13[3];
	unsigned char p23[3];
	unsigned char p33[3];
	
	get_pixel(original_bitmap, xint - 1, yint - 1, &p00[0], &p00[1], &p00[2]);
	get_pixel(original_bitmap, xint + 0, yint - 1, &p10[0], &p10[1], &p10[2]);
	get_pixel(original_bitmap, xint + 1, yint - 1, &p20[0], &p20[1], &p20[2]);
	get_pixel(original_bitmap, xint + 2, yint - 1, &p30[0], &p30[1], &p30[2]);
	
	get_pixel(original_bitmap, xint - 1, yint + 0, &p01[0], &p01[1], &p01[2]);
	get_pixel(original_bitmap, xint + 0, yint + 0, &p11[0], &p11[1], &p11[2]);
	get_pixel(original_bitmap, xint + 1, yint + 0, &p21[0], &p21[1], &p21[2]);
	get_pixel(original_bitmap, xint + 2, yint + 0, &p31[0], &p31[1], &p31[2]);
	
	get_pixel(original_bitmap, xint - 1, yint + 1, &p02[0], &p02[1], &p02[2]);
	get_pixel(original_bitmap, xint + 0, yint + 1, &p12[0], &p12[1], &p12[2]);
	get_pixel(original_bitmap, xint + 1, yint + 1, &p22[0], &p22[1], &p22[2]);
	get_pixel(original_bitmap, xint + 2, yint + 1, &p32[0], &p32[1], &p32[2]);
	
	get_pixel(original_bitmap, xint - 1, yint + 2, &p03[0], &p03[1], &p03[2]);
	get_pixel(original_bitmap, xint + 0, yint + 2, &p13[0], &p13[1], &p13[2]);
	get_pixel(original_bitmap, xint + 1, yint + 2, &p23[0], &p23[1], &p23[2]);
	get_pixel(original_bitmap, xint + 2, yint + 2, &p33[0], &p33[1], &p33[2]);
	
	unsigned char dest_pixel[3];
	int channel;
	for (channel = 0; channel < 3 /*RGB*/; channel++) {
		float col0 = cubic_hermite(p00[channel], p10[channel], p20[channel], p30[channel], xfract);
		float col1 = cubic_hermite(p01[channel], p11[channel], p21[channel], p31[channel], xfract);
		float col2 = cubic_hermite(p02[channel], p12[channel], p22[channel], p32[channel], xfract);
		float col3 = cubic_hermite(p03[channel], p13[channel], p23[channel], p33[channel], xfract);
		float value = cubic_hermite(col0, col1, col2, col3, yfract);
		if (value > 255.0f) value = 255.0f;
		if (value < 0.0f) value = 0.0f;
		dest_pixel[channel] = (unsigned char)value;
	}
	*r = dest_pixel[0];
	*g = dest_pixel[1];
	*b = dest_pixel[2];
}

void resize_image(BMP* original_bitmap, int factor, BMP* result_bitmap) {
	
	int original_height = BMP_GetHeight(original_bitmap);
	int original_width = BMP_GetWidth(original_bitmap);
	int desired_height = original_height * factor;
	int desired_width = original_width * factor;
	{
		int y;
#pragma omp parallel for schedule(static)
		for (y = 0; y < desired_height; y++) {
			float v = (float)y / (float)(desired_height - 1);
			int x;
			for (x = 0; x < desired_width; x++) {
				float u = (float)x / (float)(desired_width - 1);
				
				unsigned char r, g, b;
				bicubic_sample(original_bitmap, u, v, &r, &g, &b);
				
				//printf("x: %d, y:%d, r:%d, g:%d, b:%d\n", x, y, r, g, b);
				
				BMP_SetPixelRGB(result_bitmap, x, y, r, g, b);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("prosze podaj skale\n");
		return 4;
	}
	int factor = strtol(argv[1], NULL, 0);
	if (factor == 0) {
		printf("podaj prawidlowa skale\n");
		return 5;
	}

	BMP* bmp = BMP_ReadFile(FILE_NAME);
	 /* If an error has occurred, notify and exit */
	BMP_CHECK_ERROR( stderr, -1 );
	
	int width = BMP_GetWidth(bmp);
	int height = BMP_GetHeight(bmp);
	int depth = BMP_GetDepth(bmp);
	//print_image(bmp, width, height);
	
	printf("Szerokosc: %d, wysokosc: %d, liczba kanalow: %d\n", width, height, depth);
	
	BMP* resized_img2 = BMP_Create( width*factor, height*factor, 32 );
	resize_image(bmp, factor, resized_img2);
	
	BMP_WriteFile( resized_img2, "output-mario.bmp" );
	BMP_CHECK_ERROR( stderr, -2 );
	
	BMP_Free(bmp);
	BMP_Free(resized_img2);
	
	//resize_nearest_neighbour(data, x, y, n, factor, resized_img);
	
	return 0;
}
