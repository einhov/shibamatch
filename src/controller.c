#include <stdint.h>

#include "controller.h"

extern volatile struct {
	uint32_t DRAM_ADDR;
	uint32_t PIF_ADDR_RD64B;
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t PIF_ADDR_WR64B;
	uint32_t reserved2;
	uint32_t STATUS;
} SI;

extern const volatile struct {
	unsigned char ROM[0x7c0];
	unsigned char RAM[0x40];
} PIF;

static uint32_t pif_cmds[16] __attribute__ ((aligned(8))) = {
	0xff010401, 0xffffffff,
	0xfe000000, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 1
};

static uint16_t last_frame = 0;

void init_controller(void) {
	SI.DRAM_ADDR = (uint32_t)&pif_cmds;
	SI.PIF_ADDR_WR64B = (uint32_t)&PIF.RAM;
	while(SI.STATUS & 1);
	read_pressed_controller();
}

uint16_t read_controller(void) {
	SI.DRAM_ADDR = (uint32_t)&pif_cmds;
	SI.PIF_ADDR_RD64B = (uint32_t)&PIF.RAM;
	while(SI.STATUS & 1);
	return pif_cmds[1] >> 16;
}

uint16_t read_pressed_controller(void) {
	const uint16_t now = read_controller();
	const uint16_t result = now & ~last_frame;
	last_frame = now;
	return result;
}
