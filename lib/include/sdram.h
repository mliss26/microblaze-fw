/*
 * MicroBlaze SoC SDRAM Driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _SDRAM_H_
#define _SDRAM_H_

#include <stdint.h>


#define SDRAM_BASE              0xE0000000
#define SDRAM_SIZE              0x02000000 // 32 MiB


void sdram_pattern_test(uint32_t *sdram, uint32_t len);
void sdram_rand_d_test(uint32_t *sdram, uint32_t len, int iter);
void sdram_rand_da_test(uint32_t *sdram, uint32_t len, int iter);
void sdram_rand_log_err_counts(uint32_t *sdram, uint32_t len);

#endif /* _SDRAM_H_ */
