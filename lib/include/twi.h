/*
 * Two Wire Interface driver
 *
 * Original source: https://github.com/scttnlsn/avr-twi
 * Adapted for MicroBlaze MCS SoC on Xilinx Spartan-6
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef TWI_H
#define TWI_H

#include "xparameters.h"
#include "xiomodule.h"

#include <stdint.h>

#ifndef TWI_FREQ
#define TWI_FREQ 400000UL
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

void twi_init(XIOModule *xiomod);
void twi_write(uint8_t address, uint8_t* data, uint8_t length, void (*callback)(uint8_t, uint8_t *));
void twi_read(uint8_t address, uint8_t length, void (*callback)(uint8_t, uint8_t *));
uint8_t *twi_wait();

#endif
