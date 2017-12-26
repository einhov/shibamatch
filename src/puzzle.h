#ifndef __PUZZLE_H
#define __PUZZLE_H

#include <stdbool.h>

#include "graphics.h"

static const int PUZZLE_WIDTH = 6;
static const int PUZZLE_HEIGHT = 6;
static const int PUZZLE_SIZE = 6 * 6;

enum puzzle_state {
	PUZZLE_STATE_NONE,
	PUZZLE_STATE_ONE,
	PUZZLE_STATE_TWO,
	PUZZLE_STATE_WON
};

struct puzzle_coord {
	int x, y;
};

struct puzzle {
	struct framebuffer *fb;
	int board[6 * 6];
	bool solved[6 * 6];

	struct puzzle_coord cursor;

	struct puzzle_coord choice1;
	struct puzzle_coord choice2;

	int remaining;

	enum puzzle_state state;
};

void puzzle_init(struct puzzle *p, struct framebuffer *fb);
int puzzle_input(struct puzzle *p, uint16_t input);

#endif
