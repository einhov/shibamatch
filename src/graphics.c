#include <stdint.h>
#include <stddef.h>
#include <rcp/vi.h>

#include "graphics.h"

static vi_state_t vi_state = {
	0x0000324E, // status
	0x00200000, // origin
	0x00000140, // width
	0x00000002, // intr
	0x00000000, // current
	0x03E52239, // burst
	0x0000020D, // v_sync
	0x00000C15, // h_sync
	0x0C150C15, // leap
	0x006C02EC, // h_start
	0x002501FF, // v_start
	0x000E0204, // v_burst
	0x00000200, // x_scale
	0x00000400, // y_scale
};

static void memcpy_aligned(void *dest, void *src, size_t size) {
	for(unsigned int i = 0; i < size; i += 16) {
		__asm__ (
			".set gp=64\n\t"
			"cache 0xd, 0x0(%0)\n\t"
			"ld $t0, 0x0(%1)\n\t"
			"ld $t1, 0x8(%1)\n\t"
			"sd $t0, 0x0(%0)\n\t"
			"sd $t1, 0x8(%0)\n\t"
			"cache 0x19, 0x0(%0)\n\t"
			".set gp=default\n\t"

			:
			: "r" ((uint32_t)dest + i), "r" ((uint32_t)src + i)
			: "memory", "t0", "t1"
		);
	}
}

static void * const off_screen_buffer = (void*)0x80280000;

void init_vi(struct framebuffer *fb) {
	fb->buf = off_screen_buffer;
}

void flip(const struct framebuffer *fb) {
	vi_state.origin ^= 0x40000;
	memcpy_aligned((void*)(0x80000000 | vi_state.origin), fb->buf, 320 * 240 * 2);
	vi_flush_state(&vi_state);
}

void clear(struct framebuffer *fb) {
	for (int i = 0; i < FRAMEBUFFER_SIZE; i += 16) {
		__asm__ __volatile__(
			".set gp=64\n\t"
			"cache 0xD, 0x0(%0)\n\t"
			"sd $zero, 0x0(%0)\n\t"
			"sd $zero, 0x8(%0)\n\t"
			".set gp=default\n\t"

			:: "r" (0x80000000 | ((uint32_t)fb->buf + i))
			: "memory"
		);
	}
}

void draw_background(struct framebuffer *fb, const struct image *img) {
	for(int i = 0; i < FRAMEBUFFER_SIZE; i += 16) {
		__asm__ (
			".set gp=64\n\t"
			"cache 0xd, 0x0(%0)\n\t"
			"ld $t0, 0x0(%1)\n\t"
			"ld $t1, 0x8(%1)\n\t"
			"sd $t0, 0x0(%0)\n\t"
			"sd $t1, 0x8(%0)\n\t"
			".set gp=default\n\t"

			:
			: "r" ((uint32_t)fb->buf + i), "r" ((uint32_t)img->buf + i)
			: "memory", "t0", "t1"
		);
	}
}

void draw_image(struct framebuffer *fb, const struct image *img, int x, int y) {
	for(int i = 0; i < img->height; i++) {
		for(int j = 0; j < img->width; j++) {
			fb->buf[FRAMEBUFFER_WIDTH * (y+i) + x + j] = img->buf[img->width * i + j];
		}
	}
}

void draw_image_line_horizontal(struct framebuffer *fb, const struct image *img, int x, int y, int line) {
	for(int j = 0; j < img->width; j++) {
		fb->buf[FRAMEBUFFER_WIDTH * (y + line) + x + j] = img->buf[img->width * line + j];
	}
}

void draw_image_line_vertical(struct framebuffer *fb, const struct image *img, int x, int y, int line) {
	int i;
	for(i = 0; i < img->height; i++) {
		fb->buf[(i + y) * FRAMEBUFFER_WIDTH + x + line] = img->buf[i * img->width + line];
	}
}

void draw_cursor(struct framebuffer *fb, int x, int y) {
	const uint16_t COLOUR = 0b1111100000000001;

	int i;
	for(i = 0; i < 40; i++) {
		/* Top */
		fb->buf[(y * 40 + 0) * FRAMEBUFFER_WIDTH + x * 40 + i] = COLOUR;
		fb->buf[(y * 40 + 1) * FRAMEBUFFER_WIDTH + x * 40 + i] = COLOUR;

		/* Bottom */
		fb->buf[((y+1) * 40 - 1) * FRAMEBUFFER_WIDTH + x * 40 + i] = COLOUR;
		fb->buf[((y+1) * 40 - 2) * FRAMEBUFFER_WIDTH + x * 40 + i] = COLOUR;

		/* Left */
		fb->buf[(y * 40 + i) * FRAMEBUFFER_WIDTH + x * 40 + 0] = COLOUR;
		fb->buf[(y * 40 + i) * FRAMEBUFFER_WIDTH + x * 40 + 1] = COLOUR;

		/* Right */
		fb->buf[(y * 40 + i) * FRAMEBUFFER_WIDTH + (x+1) * 40 - 1] = COLOUR;
		fb->buf[(y * 40 + i) * FRAMEBUFFER_WIDTH + (x+1) * 40 - 2] = COLOUR;
	}

}

void blit(struct framebuffer *fb, const struct image *img, uint16_t alpha, int x, int y) {
	int lines = img->height;
	if(lines > FRAMEBUFFER_HEIGHT - y) lines = FRAMEBUFFER_HEIGHT - y;
	for(int i = 0; i < lines; i++) {
		for(int j = 0; j < img->width; j++) {
			uint16_t *pixel = &img->buf[i * img->width + j];
			if(*pixel != alpha) fb->buf[(y+i) * FRAMEBUFFER_WIDTH + (x+j)] = *pixel;
		}
	}
}
