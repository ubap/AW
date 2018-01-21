#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define FILE_NAME "mario1.gif"

void print_image(unsigned char *data, int x, int y, int n) {
	int i, j, k;
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			for (k = 0; k < n; k++) {
				unsigned char value = *(data + (i*x*n) + (j*n) + k);
				printf("data[%d][%d][%d]=%d\n", i, j, k, value);
			}
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
				memcpy(result_data + (factor*i*x*n*factor) + (factor*j*n) + (k*n),
						original_data + (i*x*n) + (j*n), n);
			}
		}
		for (k = 0; k < factor; k++) {
			// powielenie wierszy
			memcpy(result_data + (factor*i*x*n*factor) + (x*n*factor*k),
					result_data + (factor*i*x*n*factor), x*n*factor);
		}
	}
}

unsigned char* get_pixel(unsigned char* data, int height, int width,
		int channels, int x, int y) {
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	
	return data + (y*width*channels) + (x*channels);
}

float cubic_hermite(float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;
 
    return a*t*t*t + b*t*t + c*t + d;
}

void bicubic_sample(unsigned char* data, int height, int width,
		int channels, unsigned char* dest_pixel, float u, float v) {
	float x = (u * width) - 0.5; if (x<0) x = 0;
	int xint = (int)x;
	float xfract = x - floor(x);
	
	float y = (v * height) - 0.5; if (y<0) y = 0;
	int yint = (int)y;
	float yfract = y - floor(y);
	
	//printf("x: %f, xint: %d, y: %f, yint: %d\n", x, xint, y, yint);
	
	unsigned char* p00 = get_pixel(data, height, width, channels, xint - 1, yint - 1);
	unsigned char* p10 = get_pixel(data, height, width, channels, xint + 0, yint - 1);
	unsigned char* p20 = get_pixel(data, height, width, channels, xint + 1, yint - 1);
	unsigned char* p30 = get_pixel(data, height, width, channels, xint + 2, yint - 1);
	
	unsigned char* p01 = get_pixel(data, height, width, channels, xint - 1, yint + 0);
	unsigned char* p11 = get_pixel(data, height, width, channels, xint + 0, yint + 0);
	unsigned char* p21 = get_pixel(data, height, width, channels, xint + 1, yint + 0);
	unsigned char* p31 = get_pixel(data, height, width, channels, xint + 2, yint + 0);
	
	unsigned char* p02 = get_pixel(data, height, width, channels, xint - 1, yint + 1);
	unsigned char* p12 = get_pixel(data, height, width, channels, xint + 0, yint + 1);
	unsigned char* p22 = get_pixel(data, height, width, channels, xint + 1, yint + 1);
	unsigned char* p32 = get_pixel(data, height, width, channels, xint + 2, yint + 1);
	
	unsigned char* p03 = get_pixel(data, height, width, channels, xint - 1, yint + 2);
	unsigned char* p13 = get_pixel(data, height, width, channels, xint + 0, yint + 2);
	unsigned char* p23 = get_pixel(data, height, width, channels, xint + 1, yint + 2);
	unsigned char* p33 = get_pixel(data, height, width, channels, xint + 2, yint + 2);
	
	int channel;
	for (channel = 0; channel < channels; channel++) {
		float col0 = cubic_hermite(p00[channel], p10[channel], p20[channel], p30[channel], xfract);
		float col1 = cubic_hermite(p01[channel], p11[channel], p21[channel], p31[channel], xfract);
		float col2 = cubic_hermite(p02[channel], p12[channel], p22[channel], p32[channel], xfract);
		float col3 = cubic_hermite(p03[channel], p13[channel], p23[channel], p33[channel], xfract);
		float value = cubic_hermite(col0, col1, col2, col3, yfract);
		if (value > 255.0f) value = 255.0f;
		if (value < 0.0f) value = 0.0f;
		dest_pixel[channel] = (unsigned char)value;
	}
}

void resize_image(unsigned char* original_data,
		int original_width, int original_height,
		int channels, int factor, unsigned char *result_data) {
	
	int desired_height = original_height * factor;
	int desired_width = original_width * factor;

	int y, x;
#pragma omp parallel for private(x)
	for (y = 0; y < desired_height; y++) {
		float v = (float)y / (float)(desired_height - 1);
		for (x = 0; x < desired_width; x++) {
			float u = (float)x / (float)(desired_width - 1);
			unsigned char *dest_pixel = result_data + y*desired_width*channels + x*channels;
			
			bicubic_sample(original_data, original_height, original_width, channels,
					dest_pixel, u, v);
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
	
	int x,y,n;
	unsigned char *data = stbi_load(FILE_NAME, &x, &y, &n, 0);
	
	if (data == NULL) {
		printf("blad wczytywania pliku");
		return 1;
	}
	printf("Szerokosc: %d, wysokosc: %d, liczba kanalow: %d\n", x, y, n);
	//print_image(data, x, y, n);

	unsigned char *resized_img = calloc(1, x*factor*y*factor*n);
	
	//resize_nearest_neighbour(data, x, y, n, factor, resized_img);
	
	resize_image(data, x, y, n, factor, resized_img);

	stbi_write_bmp("mario1-output.bmp", factor*x, factor*y, n, resized_img);
	
	free(resized_img);
	stbi_image_free(data);
	
	return 0;
}
