/*
 * MicroBlaze SoC Global Counter Driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _GCNT_H_
#define _GCNT_H_

#include "mbsoc.h"
#include <stdint.h>


#define GCNT_HZ             XPAR_CPU_CORE_CLOCK_FREQ_HZ
#define GCNT_TICKS_PER_US   (GCNT_HZ/1000000UL)


uint64_t gcnt_get(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);


#endif /* _GCNT_H_ */
