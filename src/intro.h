#ifndef __INTRO_H
#define __INTRO_H

#include <stdbool.h>
#include <stdint.h>

#include "graphics.h"

enum intro_scene_state {
	INTRO_SCENE_STATE_BG,
	INTRO_SCENE_STATE_SHIBA,
	INTRO_SCENE_STATE_MATCH,
	INTROSCENE_STATE_IDLE
};

struct intro_scene {
	struct framebuffer *fb;
	int tick;
	enum intro_scene_state state;
};

void intro_scene_init(struct intro_scene *s, struct framebuffer *fb);
unsigned int intro_scene_frame(struct intro_scene *s);
bool intro_scene_input(struct intro_scene *s, uint16_t c);

#endif
