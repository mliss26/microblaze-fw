/*
 * MicroBlaze SoC Global Counter Driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */

#include "gcnt.h"


uint64_t gcnt_get(void)
{
    uint32_t lo, hi1, hi2;

    do {
        hi1 = GCNT_HI;
        lo  = GCNT_LO;
        hi2 = GCNT_HI;
    } while (hi1 != hi2);

    return (uint64_t)hi1 << 32 | lo;
}

void delay_us(uint32_t us)
{
    uint32_t now = GCNT_LO;
    uint32_t timeout = now + us * GCNT_TICKS_PER_US;

    // wait for wraparound
    if (timeout < now)
        while (timeout < GCNT_LO)
            ;

    // wait for expiry
    while (GCNT_LO < timeout)
        ;
}

void delay_ms(uint32_t ms)
{
    delay_us(ms * 1000);
}
