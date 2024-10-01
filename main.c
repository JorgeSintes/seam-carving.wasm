#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "stb_image.h"
#include "stb_image_write.h"

// Filters
float unit_kernel[] = {
    0, 0, 0, 0, 1, 0, 0, 0, 0,
};
float Gx[] = {-0.25, 0, 0.25, -0.5, 0, 0.5, -0.25, 0, 0.25};
float Gy[] = {-0.25, -0.5, -0.25, 0, 0, 0, 0.25, 0.5, 0.25};

float *padded_conv(uint8_t *img, float *kernel, int height, int width) {
    float *new_img = malloc(height * width * sizeof(float));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float result = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    if ((x + i) >= 0 && (x + i) < width && (y + j) >= 0 && (y + j) < height) {
                        result += img[(y + i) * width + (x + j)] * kernel[(i + 1) * 3 + j + 1];
                    }
                }
            }
            new_img[y * width + x] = result;
        }
    }
    return new_img;
}

uint8_t clamp(float x) {
    if (x > 255)
        return 255;
    else if (x < 0)
        return 0;
    return x;
}

uint8_t *sobel_filter(uint8_t *img, int height, int width) {
    uint8_t *result = malloc(height * width);
    float *Sx = padded_conv(img, Gx, height, width);
    float *Sy = padded_conv(img, Gy, height, width);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            result[y * width + x] = clamp(sqrt(pow(Sx[y * width + x], 2) + pow(Sy[y * width + x], 2)));
        }
    }
    return result;
}

uint8_t *grayscale_img(uint8_t *img, int height, int width, int n_ch) {
    uint8_t *gr_img = malloc(height * width);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y * n_ch;
            int j = x * n_ch;
            gr_img[y * width + x] =
                0.2989 * img[i * width + j] + 0.587 * img[i * width + j + 1] + 0.114 * img[i * width + j + 2];
        }
    }

    return gr_img;
}

uint32_t find_minimum_step(uint32_t *cost_matrix, int i, int j, int height, int width) {
    uint32_t min = 4294967295U;
    for (int idx = j - 1; idx <= j + 1; idx++) {
        if (cost_matrix[(i - 1) * width + idx] < min) {
            min = cost_matrix[(i - 1) * width + idx];
        }
    }
    return min;
}

uint32_t *compute_seam(uint8_t *img, int height, int width) {
    uint32_t cost_matrix[height * width];
    // Fill first row
    for (int j = 0; j < width; j++) {
        cost_matrix[j] = img[j];
    }

    // Calculate cost matrix
    for (int i = 1; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cost_matrix[i * width + j] =
                find_minimum_step(cost_matrix, i, j, height, width) + img[i * width + j];
        }
    }

    uint32_t *seam = malloc(height * sizeof(uint32_t));
    // Find initial min
    uint32_t min = 4294967295U;
    int min_idx = -1;
    for (int j = 0; j < width; j++) {
        if (cost_matrix[(height - 1) * width + j] < min) {
            min = cost_matrix[(height - 1) * width + j];
            min_idx = j;
        }
    }
    seam[height - 1] = min_idx;
    // printf("seam[%d]: %d\n", height - 1, min_idx);

    // Backtrack
    for (int i = height - 2; i >= 0; i--) {
        uint32_t min = 4294967295U;
        uint32_t upper_lim = min_idx + 1; // Need this since min_idx is being rewritten inside
        for (int j = min_idx - 1; j <= upper_lim; j++) {
            if (j >= 0 && j < width) {
                if (cost_matrix[i * width + j] < min) {
                    min = cost_matrix[i * width + j];
                    min_idx = j;
                }
            }
        }
        if (min_idx == -1) {
            printf("ERROR: Cost matrix is wrongly calculated. Couldn't find min_idx in row %d", i);
        }
        // printf("seam[%d]: %d\n", i, min_idx);
        seam[i] = min_idx;
    }

    return seam;
}

void show_seam(uint8_t *img, uint32_t *seam, int height, int width) {
    for (int i = 0; i < height; i++) {
        img[i * width + seam[i]] = 255;
    }
}

uint8_t *cut_seam(uint8_t *img, uint32_t *seam, int height, int width, int n_ch) {
    uint8_t *new_img = malloc(height * (width - 1) * n_ch);
    for (int y = 0; y < height; y++) {
        for (int x = 0, offseted_x = 0; x < width - 1; x++, offseted_x++) {
            if (x == seam[y]) {
                offseted_x++;
            }
            for (int c = 0; c < n_ch; c++) {
                new_img[(y * (width - 1) + x) * n_ch + c] = img[(y * width + offseted_x) * n_ch + c];
            }
            // printf("row %d. Copied pixel %d from %d\n", y, x, offseted_x);
        }
    }
    return new_img;
}

uint8_t *carve_image(uint8_t *input_img, int height, int width, int n_ch, int new_width) {
    uint8_t *img, *img_gr, *img_sobel, *img_cut;
    uint32_t *seam;
    img = input_img;
    for (int w = width; w > new_width; w--) {
        img_gr = grayscale_img(img, height, w, n_ch);
        img_sobel = sobel_filter(img_gr, height, w);
        seam = compute_seam(img_sobel, height, w);
        img = cut_seam(img, seam, height, w, 3);
    }
    return img;
}

int main() {
    uint8_t *img, *img_gr, *img_sobel, *img_cut;
    uint32_t *seam;
    int width, height, channels;

    img = stbi_load("input.jpg", &width, &height, &channels, 0);
    int new_width = width - 60;

    printf("Height: %d. Width: %d. Channels: %d. New width: %d\n", height, width, channels, new_width);

    img_cut = carve_image(img, height, width, channels, new_width);
    stbi_write_jpg("img_cut.jpg", new_width, height, 3, img_cut, 90);
}