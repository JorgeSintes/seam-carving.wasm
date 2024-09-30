#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stdint.h>

#include "stb_image.h"
#include "stb_image_write.h"

uint8_t *grayscale_img(uint8_t *img, int height, int width)
{
    uint8_t *gr_img = malloc(height * width);
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            gr_img[x * height + y] = 0.2989 * img[x * height + y] + 0.587 * img[x * height + y + 1] +
                                     0.114 * img[x * height + y + 2];
        }
    }

    return gr_img;
}

int main()
{
    uint8_t *img;
    uint8_t *gr_img;
    int width, height, channels;

    img = stbi_load("input.jpg", &width, &height, &channels, 1);
    printf("Height: %d. Width: %d. Channels: %d\n", height, width, channels);
    gr_img = grayscale_img(img, height, width);
    stbi_write_jpg("output.jpg", width, height, 1, gr_img, 90);
    stbi_image_free(img);
    stbi_image_free(gr_img);
}