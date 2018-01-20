#include <stdio.h>
#include <stdlib.h>

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
	print_image(data, x, y, n);
	
	
	
	unsigned char *resized_img = malloc(x*factor*y*factor*n);
	
	resize_nearest_neighbour(data, x, y, n, factor, resized_img);
	
	stbi_write_bmp("mario1-output.bmp", factor*x, factor*y, n, resized_img);
	
	stbi_image_free(data);
	return 0;
}
