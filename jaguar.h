#ifndef JAGUAR_H_
#define JAGUAR_H_

#define DRAM_WORDS (1*1024*1024)
#define GPU_RAM_BYTES 4096
#define DSP_RAM_BYTES 8192

#include "jag_video.h"

typedef struct {
	m68k_context *m68k;
	jag_video    *video;
	uint16_t     *bios;
	uint16_t     *cart;
	uint32_t     bios_size;
	uint32_t     cart_size;
	uint32_t     memcon1;
	uint32_t     memcon2;
	uint16_t     write_latch;
	uint8_t      write_pending;
	
	uint16_t     dram[DRAM_WORDS];
	uint32_t     gpu_local[GPU_RAM_BYTES / sizeof(uint32_t)];
	uint32_t     dsp_local[DSP_RAM_BYTES / sizeof(uint32_t)];
	
	uint8_t      memcon_written;
} jaguar_context;


#endif //JAGUAR_H_
