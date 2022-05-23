/*
 * INA219 I2C current/power monitor driver
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef __INA219_H__
#define __INA219_H__

#include "xparameters.h"
#include "xiomodule.h"

#include <stdbool.h>
#include <stdint.h>


#ifndef INA219_ADDR
#define INA219_ADDR         64
#endif
#ifndef INA219_CALIB
#define INA219_CALIB        4096
#endif
#ifndef INA219_TIMEOUT_MS
#define INA219_TIMEOUT_MS   100
#endif


enum ina219_reg {
    REG_CONFIG,
    REG_SHUNTV,
    REG_BUSV,
    REG_POWER,
    REG_CURRENT,
    REG_CALIB,
    REG_MAX,
};

bool ina219_init(uint8_t i2c_addr);

void ina219_set_reg(uint8_t i2c_addr, uint8_t reg_addr, uint16_t val);

uint16_t ina219_get_reg(uint8_t i2c_addr, uint8_t reg_addr);

/*
 * TODO convert to Q15.16
 */

uint16_t ina219_get_busv(uint8_t i2c_addr);

int32_t ina219_get_shuntv(uint8_t i2c_addr);

int32_t ina219_get_current(uint8_t i2c_addr);

int32_t ina219_get_power(uint8_t i2c_addr);

#endif /* __INA219_H__ */
