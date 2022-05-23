/*
 * INA219 I2C current/power monitor driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "twi.h"
#include "ina219.h"
#include "timer.h"
#include <assert.h>


enum ina219_state {
    IDLE,
    WRITE_REG,
    READ_REG_ADDR,
    READ_REG_VAL,
    ERR_TIMEOUT,
    STATE_MAX
};

static struct ina219_data_t {
    enum ina219_state   state;
    uint16_t            value;
} volatile ina_dat;

static timer_t timer;


static void ina219_i2c_cb(uint8_t i2c_addr, uint8_t *data)
{
    switch (ina_dat.state) {
        case WRITE_REG:
            ina_dat.state = IDLE;
            break;
        case READ_REG_ADDR:
            ina_dat.state = READ_REG_VAL;
            break;
        case READ_REG_VAL:
            ina_dat.value = data[1];
            ina_dat.value |= (data[0] << 8);
            ina_dat.state = IDLE;
            break;
        default:
            assert(false);
            break;
    }
}

static void ina219_timer_cb(void *data)
{
    ina_dat.state = ERR_TIMEOUT;
}

uint16_t ina219_get_reg(uint8_t i2c_addr, uint8_t reg_addr)
{
    ina_dat.state = READ_REG_ADDR;
    twi_write(i2c_addr, &reg_addr, 1, ina219_i2c_cb);
    while (ina_dat.state != READ_REG_VAL)
        ;
    twi_read(i2c_addr, 2, ina219_i2c_cb);
    while (ina_dat.state != IDLE)
        ;

    return ina_dat.value;
}

void ina219_set_reg(uint8_t i2c_addr, uint8_t reg_addr, uint16_t val)
{
    uint8_t buf[3];

    buf[0] = reg_addr;
    buf[1] = val >> 8;
    buf[2] = val & 0xff;

    ina_dat.state = WRITE_REG;
    twi_write(i2c_addr, buf, sizeof(buf), ina219_i2c_cb);
    while (ina_dat.state != IDLE)
        ;
}

uint16_t ina219_get_busv(uint8_t i2c_addr)
{
    uint16_t data = ina219_get_reg(INA219_ADDR, REG_BUSV);

    return (data >> 3) * 4; // TODO cleanup
}

int32_t ina219_get_shuntv(uint8_t i2c_addr)
{
    int16_t data = ina219_get_reg(INA219_ADDR, REG_SHUNTV);

    return (int32_t)data * 10; // TODO cleanup
}

int32_t ina219_get_current(uint8_t i2c_addr)
{
    int16_t data = ina219_get_reg(i2c_addr, REG_CURRENT);

    return (int32_t)data * 100; // TODO cleanup
}

int32_t ina219_get_power(uint8_t i2c_addr)
{
    uint16_t data = ina219_get_reg(i2c_addr, REG_POWER);
    int8_t sign = (int16_t)ina219_get_reg(i2c_addr, REG_CURRENT) >= 0 ? 1 : -1;

    return (int32_t)data * sign * 2; // TODO cleanup
}

bool ina219_init(uint8_t i2c_addr)
{
    timer_init(&timer, TIMER_ONE_SHOT, ina219_timer_cb, NULL);

    ina219_set_reg(INA219_ADDR, REG_CALIB, INA219_CALIB);
    if (ina219_get_reg(INA219_ADDR, REG_CALIB) != INA219_CALIB)
        return false;

    return true;
}
