#include "graphics.h"
#include "victory.h"

static const double sine_lut[64] = {
	0x0.0000000000000p+0, 0x1.92155f7a3667ep-6, 0x1.91f65f10dd814p-5, 0x1.2d52092ce19f6p-4,
	0x1.917a6bc29b42cp-4, 0x1.f564e56a9730ep-4, 0x1.2c8106e8e613ap-3, 0x1.5e214448b3fc6p-3,
	0x1.8f8b83c69a60ap-3, 0x1.c0b826a7e4f63p-3, 0x1.f19f97b215f1ap-3, 0x1.111d262b1f677p-2,
	0x1.294062ed59f05p-2, 0x1.4135c94176602p-2, 0x1.58f9a75ab1fddp-2, 0x1.7088530fa459ep-2,
	0x1.87de2a6aea963p-2, 0x1.9ef7943a8ed8ap-2, 0x1.b5d1009e15cc0p-2, 0x1.cc66e9931c45dp-2,
	0x1.e2b5d3806f63bp-2, 0x1.f8ba4dbf89abap-2, 0x1.073879922ffedp-1, 0x1.11eb3541b4b22p-1,
	0x1.1c73b39ae68c8p-1, 0x1.26d054cdd12dfp-1, 0x1.30ff7fce17035p-1, 0x1.3affa292050b9p-1,
	0x1.44cf325091dd6p-1, 0x1.4e6cabbe3e5e9p-1, 0x1.57d69348cec9fp-1, 0x1.610b7551d2cdep-1,
	0x1.6a09e667f3bccp-1, 0x1.72d0837efff96p-1, 0x1.7b5df226aafafp-1, 0x1.83b0e0bff976dp-1,
	0x1.8bc806b151741p-1, 0x1.93a22499263fbp-1, 0x1.9b3e047f38740p-1, 0x1.a29a7a0462782p-1,
	0x1.a9b66290ea1a3p-1, 0x1.b090a581501ffp-1, 0x1.b728345196e3ep-1, 0x1.bd7c0ac6f9529p-1,
	0x1.c38b2f180bdb0p-1, 0x1.c954b213411f5p-1, 0x1.ced7af43cc773p-1, 0x1.d4134d14dc93ap-1,
	0x1.d906bcf328d46p-1, 0x1.ddb13b6ccc23cp-1, 0x1.e212104f686e5p-1, 0x1.e6288ec48e112p-1,
	0x1.e9f4156c62ddbp-1, 0x1.ed740e7684963p-1, 0x1.f0a7efb9230d7p-1, 0x1.f38f3ac64e589p-1,
	0x1.f6297cff75cb0p-1, 0x1.f8764fa714ba9p-1, 0x1.fa7557f08a517p-1, 0x1.fc26470e19fd3p-1,
	0x1.fd88da3d12525p-1, 0x1.fe9cdad01883ap-1, 0x1.ff621e3796d7ep-1, 0x1.ffd886084cd0dp-1
};

static double sine(uint16_t x) {
	switch(x & 0xc000) {
		case 0x4000:
			x = 0x7fff - x;
			__attribute__((fallthrough));
		case 0x0000:
			return sine_lut[x >> 8];

		case 0xc000:
			x = 0xffff - x;
			__attribute__((fallthrough));
		case 0x8000:
			x &= 0x3fff;
			return -sine_lut[x >> 8];
	}
	__builtin_unreachable();
}

extern char _binary_images_doge_raw_start[];
extern char _binary_images_balloons_raw_start[];
static const struct image victory_doge =
	{ .buf = (uint16_t*)_binary_images_doge_raw_start, .width = 320, .height = 240 };
static const struct image balloons =
	{ .buf = (uint16_t*)_binary_images_balloons_raw_start, .width = 39, .height = 80 };

void victory_scene_init(struct victory_scene *v, struct framebuffer *fb) {
	v->fb = fb;
	v->tick = 0;
	v->state = VICTORY_SCENE_STATE_BG;
}

static void blit_repair_background(struct framebuffer *fb, int x1, int y1, int w, int h, int x2, int y2) {
	if(y1 + h > FRAMEBUFFER_HEIGHT) h = FRAMEBUFFER_HEIGHT - y1;
	for(int i = 0; i < h; i++) {
		if(y1 + i >= y2 + h - 1 || y1 + i < y2) {
			for(int j = x1; j < x1 + w * 2; j++) {
				fb->buf[(y1+i) * FRAMEBUFFER_WIDTH + j] = victory_doge.buf[(y1+i) * FRAMEBUFFER_WIDTH + j];
			}
		} else {
			for(int j = 0; j < w; j++) {
				if(((x1 + j >= x2 + w) || (x1 + j < x2)) || (
					(balloons.buf[i * w + j] != 1) &&
					(balloons.buf[(i + (y1 - y2)) * w + (x1 - x2) + j] == 1)
					)) {
					fb->buf[(y1+i) * FRAMEBUFFER_WIDTH + x1 + j] =
						victory_doge.buf[(y1+i) * FRAMEBUFFER_WIDTH + x1 + j];
				}
			}
		}
	}
}

unsigned int victory_scene_frame(struct victory_scene *v) {
	static const uint16_t fac = 0x800;

	switch(v->state) {
		case VICTORY_SCENE_STATE_BG:
			draw_image(v->fb, &victory_doge, 0, 0);
			v->state = VICTORY_SCENE_STATE_BALLOON;
			return 45;

		case VICTORY_SCENE_STATE_BALLOON: {
			if(v->tick < 180) {
				int x1 = 40 + sine(fac * (v->tick / 6.)) * 10;
				int y1 = 240 - v->tick;
				v->tick++;
				int x2 = 40 + sine(fac * (v->tick / 6.)) * 10;
				int y2 = 240 - v->tick;
				blit_repair_background(v->fb, x1, y1, balloons.width, balloons.height, x2, y2);
				blit(v->fb, &balloons, 1, x2, y2);
			} else {
				v->tick--;
				v->state = VICTORY_SCENE_STATE_IDLE;
			}
			return 1;
		}

		case VICTORY_SCENE_STATE_IDLE: {
			if(v->tick == 179) draw_image(v->fb, &victory_doge, 0, 0);
			int x1 = 40 + sine(fac * (v->tick / 6.)) * 10;
			int y1 = 240 - 180 + sine(fac * (v->tick / 4.)) * 3;
			v->tick++;
			int x2 = 40 + sine(fac * (v->tick / 6.)) * 10;
			int y2 = 240 - 180 + sine(fac * (v->tick / 4.)) * 3;
			blit_repair_background(v->fb, x1, y1, balloons.width, balloons.height, x2, y2);
			blit(v->fb, &balloons, 1, x2, y2);
			return 1;
		}
	}
	return 1000000;
}

bool victory_scene_input(struct victory_scene *v, uint16_t input) {
	if(input && (v->state == VICTORY_SCENE_STATE_IDLE || v->state == VICTORY_SCENE_STATE_BALLOON)) return true;
	else return false;
}
