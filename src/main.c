/*
 * MicroBlaze MCS SoC firmware entrypoint
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "mbsoc.h"
#include "ina219.h"
#include "lcd.h"
#include "sdram.h"
#include "timer.h"
#include "twi.h"
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

static void ina219_dump_regs(uint8_t i2c_addr)
{
    uint16_t data;
    uint8_t i;

    xil_printf("                    reg: value\r\n");
    for (i = 0; i < REG_MAX; ++i) {
        data = ina219_get_reg(i2c_addr, i);
        xil_printf("                    % 3d: 0x%04x\r\n", i, data);
    }
}

static void ina219_dump_sample(void)
{
    uint64_t tick = gcnt_get();
    struct data_sample {
        uint16_t    busv;
        int32_t     shuntv;
        int32_t     current;
        int32_t     power;
    } sample = { 0 };

    sample.busv = ina219_get_busv(INA219_ADDR);
    sample.shuntv = ina219_get_shuntv(INA219_ADDR);
    sample.current = ina219_get_current(INA219_ADDR);
    sample.power = ina219_get_power(INA219_ADDR);

    lcd_clr();
    lcd_home();

    uint16_t vw = sample.busv / 1000;
    uint16_t vd = sample.busv / 100 - (vw * 10);
    lcd_putui(vw, 0);
    lcd_putch('.');
    lcd_putui(vd, 1);
    lcd_putch(' ');
    lcd_putch('V');
    lcd_putch(' ');

    int32_t pw = sample.power / 1000;
    uint32_t pd = abs(sample.power - (pw * 1000));
    if (sample.power < 0)
        lcd_putch('-');
    lcd_putui(pw, 0);
    lcd_putch('.');
    lcd_putui(pd, 3);
    lcd_putch(' ');
    lcd_putch('m');
    lcd_putch('W');
    lcd_putch('\n');

    int32_t iw = sample.current / 1000000;
    uint32_t id = abs(sample.current / 100 - (iw * 10000));
    if (sample.current < 0)
        lcd_putch('-');
    lcd_putui(iw, 0);
    lcd_putch('.');
    lcd_putui(id, 4);
    lcd_putch(' ');
    lcd_putch('A');

    xil_printf("0x%06x%08x: bus (mV): %d \t", (uint32_t)(tick>>32), (uint32_t)tick, sample.busv);
    xil_printf("shunt (uV): %ld   \t", sample.shuntv);
    xil_printf("current (uA): %ld \t", sample.current);
    xil_printf("power (mW): %ld\r\n", sample.power);
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

    twi_init(&xio);

    // enable global interrupts last
    microblaze_enable_interrupts();

    return rv;
}

int main()
{
    bool status;
    uint32_t *sdram = (uint32_t *)SDRAM_BASE;

    assert(mb_init() == XST_SUCCESS);
    log("MicroBlaze CPU/IO Freq: %d MHz", XPAR_MICROBLAZE_FREQ/1000000UL);
    log("mbsoc starting...");

    lcd_init();
    lcd_config(LCD_CFG_BACKLIGHT_ON | LCD_CFG_DISPLAY_ON);
    lcd_clr();
    lcd_puts("ram test...");

    sdram_pattern_test(sdram, SDRAM_SIZE);
    sdram_rand_d_test(sdram, SDRAM_SIZE, 1);
    sdram_rand_da_test(sdram, SDRAM_SIZE, 1);

    status = ina219_init(INA219_ADDR);
    log("ina219_init %s", status ? "success" : "failed");
    ina219_dump_regs(INA219_ADDR);

    //sdram_rand_log_err_counts(sdram, SDRAM_SIZE);

    log("system init complete");
    while (true)
    {
        ina219_dump_sample();
        delay_ms(200);
    }

    return 0;
}
