#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
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

float *padded_conv(uint8_t *img, float *kernel, int height, int width)
{
    float *new_img = malloc(height * width * sizeof(float));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float result = 0;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    if ((x + i) >= 0 && (x + i) < width && (y + j) >= 0 && (y + j) < height)
                    {
                        result += img[(y + i) * width + (x + j)] * kernel[(i + 1) * 3 + j + 1];
                    }
                }
            }
            new_img[y * width + x] = result;
        }
    }
    return new_img;
}

uint8_t clamp(float x)
{
    if (x > 255)
        return 255;
    else if (x < 0)
        return 0;
    return x;
}

uint8_t *sobel_filter(uint8_t *img, int height, int width)
{
    uint8_t *result = malloc(height * width);
    float *Sx = padded_conv(img, Gx, height, width);
    float *Sy = padded_conv(img, Gy, height, width);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            result[y * width + x] = clamp(sqrt(pow(Sx[y * width + x], 2) + pow(Sy[y * width + x], 2)));
        }
    }
    return result;
}

uint8_t *grayscale_img(uint8_t *img, int height, int width, int n_ch)
{
    uint8_t *gr_img = malloc(height * width);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = y * n_ch;
            int j = x * n_ch;
            gr_img[y * width + x] =
                0.2989 * img[i * width + j] + 0.587 * img[i * width + j + 1] + 0.114 * img[i * width + j + 2];
        }
    }

    return gr_img;
}

int main()
{
    uint8_t *img, *gr_img, *new_img;
    int width, height, channels;

    img = stbi_load("input.jpg", &width, &height, &channels, 0);
    printf("Height: %d. Width: %d. Channels: %d\n", height, width, channels);
    gr_img = grayscale_img(img, height, width, channels);
    new_img = sobel_filter(gr_img, height, width);

    stbi_write_jpg("img_gr.jpg", width, height, 1, gr_img, 90);
    stbi_write_jpg("img_sobel.jpg", width, height, 1, new_img, 90);

    stbi_image_free(img);
    stbi_image_free(gr_img);
    stbi_image_free(new_img);
}