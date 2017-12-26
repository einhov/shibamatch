#include <stdint.h>

#include <libn64.h>
#include <syscall.h>

#include "controller.h"
#include "intro.h"
#include "puzzle.h"
#include "victory.h"

enum game_state {
	GAME_STATE_INTRO,
	GAME_STATE_INGAME,
	GAME_STATE_VICTORY
};

static struct framebuffer fb;
static struct intro_scene i;
static struct puzzle p;
static struct victory_scene v;

void main(void *dummy __attribute__((unused))) {
	init_controller();
	init_vi(&fb);
	libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);

	enum game_state state = GAME_STATE_INTRO;

	switch(read_controller()) {
		case 0x10:
			state = GAME_STATE_INGAME;
			break;
		case 0x20:
			state = GAME_STATE_VICTORY;
			break;
	}

	switch(state) {
		case GAME_STATE_INTRO:
			intro_scene_init(&i, &fb);
			break;
		case GAME_STATE_INGAME:
			puzzle_init(&p, &fb);
			break;
		case GAME_STATE_VICTORY:
			victory_scene_init(&v, &fb);
			break;
	}

	for(;;) {
		int wait_frames = 0;
		switch(state) {
			case GAME_STATE_INTRO:
				wait_frames = intro_scene_frame(&i);
				break;
			case GAME_STATE_INGAME:
				wait_frames = 1;
				break;
			case GAME_STATE_VICTORY:
				wait_frames = victory_scene_frame(&v);
				break;
		}

		flip(&fb);
		while(wait_frames--) {
			libn64_recv_message();
		}

		const uint16_t input = read_pressed_controller();
		switch(state) {
			case GAME_STATE_INTRO:
				if(intro_scene_input(&i, input)) {
					puzzle_init(&p, &fb);
					state = GAME_STATE_INGAME;
				}
				break;
			case GAME_STATE_INGAME:
				if(puzzle_input(&p, input) <= 0) {
					victory_scene_init(&v, &fb);
					state = GAME_STATE_VICTORY;
				}
				break;
			case GAME_STATE_VICTORY:
				if(victory_scene_input(&v, input)) {
					state = GAME_STATE_INTRO;
					intro_scene_init(&i, &fb);
				}
				break;
		}
	}
}
