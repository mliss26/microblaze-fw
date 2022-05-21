/*
 * MicroBlaze SoC SDRAM Driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "ubsoc.h"
#include "sdram.h"
#include "util.h"
#include <stdbool.h>
#include <stdint.h>


void sdram_pattern_test(uint32_t *sdram, uint32_t len)
{
    uint32_t value;
    uint32_t patterns[] = {
        0xa55aa55a, 0x5aa55aa5,
        0x55aa55aa, 0xaa55aa55,
        0xffffffff, 0x00000000,
        0xff00ff00, 0x00ff00ff,
    };

    log("beginning SDRAM pattern test...");

    for (int j = 0; j < ARRAY_SIZE(patterns); ++j) {
        log("pattern: 0x%08x", patterns[j]);

        for (int i = 0; i < len/4; ++i) {
            sdram[i] = patterns[j];
        }

        for (int i = 0; i < len/4; ++i) {
            value = sdram[i];
            if (value != patterns[j])
                log("    error at 0x%08x: 0x%08x", &sdram[i], value);
        }
    }
}

void sdram_rand_d_test(uint32_t *sdram, uint32_t len, int iter)
{
    uint32_t value, expected;

    log("beginning SDRAM random data, sequential write/read test...");

    PRNG_SEED = 0;
    for (int j = 0; j < iter; ++j)
    {
        log("PRNG seed: 0x%08x", PRNG_SEED);
        for (int i = 0; i < len/4; ++i) {
            sdram[i] = PRNG_RAND;
        }

        log("write complete, verifying...");
        PRNG_SEED = PRNG_SEED;
        for (int i = 0; i < len/4; ++i) {
            value = sdram[i];
            expected = PRNG_RAND;
            if (value != expected)
                log("    error at 0x%08x: expected 0x%08x, actual 0x%08x (0x%08x)", &sdram[i], expected, value, expected ^ value);
        }

        PRNG_SEED = PRNG_SEED + 1;
    }
}

void sdram_rand_da_test(uint32_t *sdram, uint32_t len, int iter)
{
    uint32_t value, expected, addr_i;

    log("beginning SDRAM random data/address test...");

    PRNG_SEED = GCNT_LO;
    for (int j = 0; j < iter; ++j)
    {
        log("PRNG seed: 0x%08x", PRNG_SEED);

        for (int i = 0; i < len/4; ++i)
        {
            addr_i = PRNG_RAND & (SDRAM_SIZE-1);
            expected = PRNG_RAND;
            sdram[addr_i] = expected;
            value = sdram[addr_i];
            if (value != expected)
                log("    error at 0x%08x: expected 0x%08x, actual 0x%08x (0x%08x)", &sdram[i], expected, value, expected ^ value);
        }

        PRNG_SEED = PRNG_RAND;
    }
}

static void print_bit_errors(uint32_t bit_errors[32])
{
    for (int i = 0; i < 32; ++i) {
        if (bit_errors[i] == 0)
            continue;
        xil_printf("%db: %d ", i+1, bit_errors[i]);
    }
    xil_printf("\r\n");
}

void sdram_rand_log_err_counts(uint32_t *sdram, uint32_t len)
{
    uint32_t value, expected, addr_i, tmp;
    uint32_t bit_errors[32] = {0};

    log("beginning SRAM random data/address test with error count logging...");

    while (true)
    {
        PRNG_SEED = PRNG_SEED + 1;
        log("PRNG seed: 0x%08x", PRNG_SEED);

        for (int i = 0; i < len/4; ++i)
        {
            addr_i = PRNG_RAND & (SDRAM_SIZE-1);
            expected = PRNG_RAND;
            sdram[addr_i] = expected;
            value = sdram[addr_i];
            if (value != expected)
            {
                tmp = __builtin_popcount(value ^ expected);
                if (tmp > 32) {
                    log("ERROR: __builtin_popcount returned illegal value 0x%x", tmp);
                } else {
                    bit_errors[tmp-1]++;
                    xil_printf("0x%08x ", value ^ expected);
                    print_bit_errors(bit_errors);
                }
            }
        }
    }
}
