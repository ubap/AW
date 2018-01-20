#include <stdio.h>
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

int main() {
	int x,y,n;
	unsigned char *data = stbi_load(FILE_NAME, &x, &y, &n, 0);
	
	if (data == NULL) {
		printf("blad wczytywania pliku");
		return 1;
	}
	printf("Szerokosc: %d, wysokosc: %d, liczba kanalow: %d\n", x, y, n);
	print_image(data, x, y, n);
	
	
	stbi_write_bmp("mario1-output.bmp", x, y, n, data);
	
	stbi_image_free(data);
	return 0;
}
