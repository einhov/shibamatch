#include <stdint.h>
#include "puzzle.h"

#define RAW(name) extern char _binary_images_##name##_raw_start[]

RAW(1); RAW(2); RAW(3); RAW(4); RAW(5); RAW(6); RAW(7); RAW(8); RAW(9); RAW(10);
RAW(11); RAW(12); RAW(13); RAW(14); RAW(15); RAW(16); RAW(17); RAW(18);
RAW(flipped); RAW(sidebar);

#define DOGE_TILE(name) \
	{ .buf = (uint16_t*)_binary_images_##name##_raw_start, .width = 40, .height = 40 }

static const struct image doges[] = {
	DOGE_TILE(1), DOGE_TILE(2), DOGE_TILE(3), DOGE_TILE(4),
	DOGE_TILE(5), DOGE_TILE(6), DOGE_TILE(7), DOGE_TILE(8),
	DOGE_TILE(9), DOGE_TILE(10), DOGE_TILE(11), DOGE_TILE(12),
	DOGE_TILE(13), DOGE_TILE(14), DOGE_TILE(15), DOGE_TILE(16),
	DOGE_TILE(17), DOGE_TILE(18)
};

static const struct image flipped = DOGE_TILE(flipped);
static const struct image sidebar =
	{ .buf = (uint16_t*)_binary_images_sidebar_raw_start, .width = 80, .height = 240 };

static void draw_tile(const struct puzzle *p, const struct puzzle_coord coord) {
	draw_image(p->fb,
		&doges[p->board[coord.y * PUZZLE_WIDTH + coord.x]], 80 + coord.x * 40, coord.y * 40);
}

static void draw_flipped_tile(const struct puzzle *p, const struct puzzle_coord coord) {
	draw_image(p->fb, &flipped, 80 + coord.x * 40, coord.y * 40);
}

static void refresh_tile(const struct puzzle *p, const struct puzzle_coord coord) {
	if(p->solved[coord.y * PUZZLE_WIDTH + coord.x] == true)
		draw_tile(p, coord);
	else if(p->state != PUZZLE_STATE_NONE && (
		(p->choice1.x == coord.x && p->choice1.y == coord.y) ||
		(p->choice2.x == coord.x && p->choice2.y == coord.y)))
		draw_tile(p, coord);

	else
		draw_flipped_tile(p, coord);
}

static void draw_full_board(const struct puzzle *p) {
	int i, j;
	for(i = 0; i < PUZZLE_WIDTH; i++)
		for(j = 0; j < PUZZLE_HEIGHT; j++)
			refresh_tile(p, (struct puzzle_coord) {i, j});
}

static void xchg(int *a, int *b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

static unsigned int rand_seed = 0;

static void srand(unsigned int seed) {
	rand_seed = seed;
}

static int rand(void) {
	static const unsigned int fac1 = 0x0019660d;
	static const unsigned int fac2 = 0x3c6ef35f;
	rand_seed = (rand_seed * fac1) + fac2;
	return rand_seed & ~0x80000000;
}

static unsigned int cp0_count(void) {
	unsigned int count;
	__asm__ ("mfc0 %0, $9" : "=r" (count));
	return count;
}

void puzzle_init(struct puzzle *p, struct framebuffer *fb) {
	p->fb = fb;
	p->cursor.x = 0;
	p->cursor.y = 0;

	for(int i = 0; i < PUZZLE_SIZE; i++) {
		p->board[i] = i % (PUZZLE_SIZE / 2);
		p->solved[i] = false;
	}

	srand(cp0_count());

	// Modified Fisher-Yates shuffle
	for(int i = PUZZLE_SIZE - 1; i >= 1; i--) {
		int j = rand() % (i + 1);
		xchg(&p->board[i], &p->board[j]);
	}

	p->choice1.x = -1;
	p->choice1.y = -1;
	p->choice2.x = -1;
	p->choice2.y = -1;
	p->remaining = PUZZLE_SIZE / 2;
	p->state = PUZZLE_STATE_NONE;

	clear(p->fb);
	draw_image(p->fb, &sidebar, 0, 0);
	draw_full_board(p);
	draw_cursor(p->fb, p->cursor.x + 2, p->cursor.y);
}

static int do_move(struct puzzle *p, struct puzzle_coord t1, struct puzzle_coord t2) {
	if(p->board[t1.y * PUZZLE_WIDTH + t1.x] == p->board[t2.y * PUZZLE_WIDTH + t2.x]) {
		p->solved[t1.y * PUZZLE_WIDTH + t1.x] = true;
		p->solved[t2.y * PUZZLE_WIDTH + t2.x] = true;
		p->remaining--;
	}
	return p->remaining;
}

int puzzle_input(struct puzzle *p, uint16_t input) {
	if(input == 0) return p->remaining;

	if(p->state == PUZZLE_STATE_TWO) {
		p->state = PUZZLE_STATE_NONE;
		refresh_tile(p, p->choice1);
		refresh_tile(p, p->choice2);
		p->choice1.x = -1;
		p->choice1.y = -1;
		p->choice2.x = -1;
		p->choice2.y = -1;

		// Redraw cursor in case the cursor position doesn't move
		draw_cursor(p->fb, p->cursor.x + 2, p->cursor.y);
	}

	// Debug win with simultaneous press of Z and B
	if((input & 0x6000) == 0x6000) return 0;

	int old_x = p->cursor.x;
	int old_y = p->cursor.y;

	// DPAD cursor movement
	if(input & 0x200) p->cursor.x--;
	if(input & 0x100) p->cursor.x++;
	if(input & 0x800) p->cursor.y--;
	if(input & 0x400) p->cursor.y++;
	if(p->cursor.x < 0) p->cursor.x = 0;
	if(p->cursor.y < 0) p->cursor.y = 0;
	if(p->cursor.x > 5) p->cursor.x = 5;
	if(p->cursor.y > 5) p->cursor.y = 5;

	// A button to select a tile
	if(input & 0x8000) {
		if(p->state == PUZZLE_STATE_NONE) {
			if(!p->solved[p->cursor.y * PUZZLE_WIDTH + p->cursor.x]) {
				p->choice1.x = p->cursor.x;
				p->choice1.y = p->cursor.y;
				p->state = PUZZLE_STATE_ONE;
			}
		} else if(p->state == PUZZLE_STATE_ONE) {
			if(((p->choice1.x != p->cursor.x) || (p->choice1.y != p->cursor.y)) &&
				(!p->solved[p->cursor.y * PUZZLE_WIDTH + p->cursor.x])) {
				p->choice2.x = p->cursor.x;
				p->choice2.y = p->cursor.y;

				do_move(p, p->choice1, p->choice2);

				p->state = PUZZLE_STATE_TWO;
			}
		}

		refresh_tile(p, p->cursor);
		draw_cursor(p->fb, p->cursor.x + 2, p->cursor.y);
	}

	if(p->cursor.x == old_x && p->cursor.y == old_y) return p->remaining;

	// Redraw over old cursor
	refresh_tile(p, (struct puzzle_coord) { old_x, old_y });

	// Draw new cursor
	draw_cursor(p->fb, p->cursor.x + 2, p->cursor.y);
	return p->remaining;
}
