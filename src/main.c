/*
 * MicroBlaze MCS SoC firmware entrypoint
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "mbsoc.h"
#include "sdram.h"
#include "timer.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>


#define STDOUT_BAUD     460800


/*
 * Global XIO module for BSP
 */
XIOModule xio;

/*
 * LED heartbeat
 */
static timer_t hb_timer;

static void heartbeat(void *data)
{
    static uint8_t leds = 0;

    LEDS = leds++;
}

/*
 * Initialize the BSP and system
 */
static int mb_init(void)
{
    int rv;

    rv = XIOModule_Initialize(&xio, XPAR_IOMODULE_0_DEVICE_ID);
    if (rv != XST_SUCCESS)
        return -1;

    rv = XIOModule_SetBaudRate(&xio, STDOUT_BAUD);
    if (rv != XST_SUCCESS)
        return -1;

    xil_printf("\r\n\r\n");

    // register the XIO module driver interrupt handler with processor
    microblaze_register_handler(XIOModule_DeviceInterruptHandler, XPAR_IOMODULE_0_DEVICE_ID);
    XIOModule_Start(&xio);

    // initialize timer services
    timer_svc_init();

    timer_init(&hb_timer, TIMER_PERIODIC, heartbeat, NULL);
    timer_set(&hb_timer, TIMEOUT_IN_MS(250));

    // enable global interrupts last
    microblaze_enable_interrupts();

    return rv;
}

int main()
{
    uint32_t *sdram = (uint32_t *)SDRAM_BASE;

    assert(mb_init() == XST_SUCCESS);
    log("MicroBlaze CPU/IO Freq: %d MHz", XPAR_MICROBLAZE_FREQ/1000000UL);
    log("mbsoc starting...");

    sdram_pattern_test(sdram, SDRAM_SIZE);
    sdram_rand_d_test(sdram, SDRAM_SIZE, 1);
    sdram_rand_da_test(sdram, SDRAM_SIZE, 1);

    sdram_rand_log_err_counts(sdram, SDRAM_SIZE);

    // main loop never reached with memory tests
    while (true)
    {
    }

    return 0;
}
