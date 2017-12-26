#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <stdint.h>

static const int FRAMEBUFFER_WIDTH = 320;
static const int FRAMEBUFFER_HEIGHT = 240;
static const int FRAMEBUFFER_DEPTH = 2;
static const int FRAMEBUFFER_SIZE = 320 * 240 * 2;

struct framebuffer {
	uint16_t *buf;
};

struct image {
	int width;
	int height;
	uint16_t *buf;
};

void init_vi(struct framebuffer *fb);
void flip(const struct framebuffer *fb);
void clear(struct framebuffer *fb);
void draw_background(struct framebuffer *fb, const struct image *img);
void draw_image(struct framebuffer *fb, const struct image *img, int x, int y);
void draw_image_line_horizontal(struct framebuffer *fb, const struct image *img, int x, int y, int line);
void draw_image_line_vertical(struct framebuffer *fb, const struct image *img, int x, int y, int line);
void draw_cursor(struct framebuffer *fb, int x, int y);
void blit(struct framebuffer *fb, const struct image *img, uint16_t alpha, int x, int y);

#endif
