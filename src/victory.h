#ifndef __VICTORY_H
#define __VICTORY_H

#include <stdbool.h>

#include "graphics.h"

enum victory_scene_state {
	VICTORY_SCENE_STATE_BG,
	VICTORY_SCENE_STATE_BALLOON,
	VICTORY_SCENE_STATE_IDLE
};

struct victory_scene {
	struct framebuffer *fb;
	int tick;
	enum victory_scene_state state;
};

void victory_scene_init(struct victory_scene *v, struct framebuffer *fb);
unsigned int victory_scene_frame(struct victory_scene *v);
bool victory_scene_input(struct victory_scene *v, uint16_t input);

#endif
