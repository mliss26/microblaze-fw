/*
 * Hexdump functions for debugging
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */

#include "hexdump.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>


void hexdump32(void const *addr, size_t len)
{
    uint32_t const *data = addr;

    for (size_t i = 0; i < len; i += HEXDUMP_LINE_BYTES/sizeof(*data), data++) {
        if ((uintptr_t)data % HEXDUMP_LINE_BYTES == 0)
            xil_printf("0x%08x:", (uintptr_t)data);

        xil_printf(" %08x", *data);

        if (((uintptr_t)data % HEXDUMP_LINE_BYTES) == (HEXDUMP_LINE_BYTES - sizeof(*data)))
            xil_printf("\r\n");
    }
    if (((uintptr_t)data % HEXDUMP_LINE_BYTES) != 0)
        xil_printf("\r\n");
}
