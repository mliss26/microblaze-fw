/*
 * Two Wire Interface driver
 *
 * Original source: https://github.com/scttnlsn/avr-twi
 * Adapted for MicroBlaze MCS SoC on Xilinx Spartan-6
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "twi.h"
#include "xparameters.h"
#include "xiomodule.h"
#include "oc_i2c_master.h"

#include <string.h>


#define TW_READ		1
#define TW_WRITE	0


enum oc_i2c_state {
    I2C_IDLE,
    I2C_ADDR_WAIT,
    I2C_TX_WAIT,
    I2C_RX_WAIT,
    I2C_DONE,
    I2C_STATE_MAX
};


static volatile uint8_t busy;
static struct {
    uint8_t state;
    uint8_t buffer[TWI_BUFFER_LENGTH];
    uint8_t length;
    uint8_t index;
    void (*callback)(uint8_t, uint8_t *);
} transmission;


void twi_isr(void *data);


void twi_init(XIOModule *xio) {
    uint16_t prescale = (XPAR_CPU_CORE_CLOCK_FREQ_HZ / (5 * TWI_FREQ)) - 1;

    busy = 0;
    transmission.state = I2C_IDLE;
    memset(transmission.buffer, 0, TWI_BUFFER_LENGTH);

    XIOModule_Connect(xio, OC_I2C_IRQ, twi_isr, NULL);
    XIOModule_Enable(xio, OC_I2C_IRQ);

    OC_I2C_REG(PRER_LO) = (uint8_t)prescale;
    OC_I2C_REG(PRER_HI) = (uint8_t)(prescale>>8);
    OC_I2C_REG(CTR) = OC_I2C_EN | OC_I2C_IEN;
}

uint8_t *twi_wait(void) {
    while (busy)
        ;
    return &transmission.buffer[1];
}

void twi_done(void) {
    uint8_t address = transmission.buffer[0] >> 1;
    uint8_t *data = &transmission.buffer[1];

    busy = 0;

    if (transmission.callback != NULL) {
        transmission.callback(address, data);
    }
}

void twi_write(uint8_t address, uint8_t* data, uint8_t length, void (*callback)(uint8_t, uint8_t *)) {
    twi_wait();

    busy = 1;

    transmission.buffer[0] = (address << 1) | TW_WRITE;
    transmission.length = length + 1;
    transmission.index = 1;
    transmission.callback = callback;
    memcpy(&transmission.buffer[1], data, length);

    transmission.state = I2C_TX_WAIT;
    OC_I2C_REG(TXR) = transmission.buffer[0];
    OC_I2C_REG(CR) = OC_I2C_STA | OC_I2C_WR;
}

void twi_read(uint8_t address, uint8_t length, void (*callback)(uint8_t, uint8_t *)) {
    twi_wait();

    busy = 1;

    transmission.buffer[0] = (address << 1) | TW_READ;
    transmission.length = length + 1;
    transmission.index = 1;
    transmission.callback = callback;

    transmission.state = I2C_ADDR_WAIT;
    OC_I2C_REG(TXR) = transmission.buffer[0];
    OC_I2C_REG(CR) = OC_I2C_STA | OC_I2C_WR;
}

void twi_isr(void *data)
{
    uint8_t sr = OC_I2C_REG(SR); // cache status register with ACK/NACK

    OC_I2C_REG(CR) = OC_I2C_IACK; // ack interrupt

    switch (transmission.state) {
        case I2C_IDLE:
            break;
        case I2C_TX_WAIT:
            if (sr & OC_I2C_RXACK) { // NACK
                OC_I2C_REG(CR) = OC_I2C_STO;
                transmission.state = I2C_DONE;
            } else { // ACK
                if (transmission.index < transmission.length) {
                    OC_I2C_REG(TXR) = transmission.buffer[transmission.index];
                    if (transmission.index == (transmission.length - 1)) {
                        OC_I2C_REG(CR) = OC_I2C_STO | OC_I2C_WR;
                        transmission.state = I2C_DONE;
                    } else {
                        OC_I2C_REG(CR) = OC_I2C_WR;
                    }
                    ++transmission.index;
                }
            }
            break;
        case I2C_ADDR_WAIT:
            if (sr & OC_I2C_RXACK) { // NACK
                OC_I2C_REG(CR) = OC_I2C_STO;
                transmission.state = I2C_DONE;
            } else { // ACK
                if (transmission.index == (transmission.length - 1))
                    OC_I2C_REG(CR) = OC_I2C_RD | OC_I2C_ACK | OC_I2C_STO;
                else
                    OC_I2C_REG(CR) = OC_I2C_RD;
                transmission.state = I2C_RX_WAIT;
            }
            break;
        case I2C_RX_WAIT:
            transmission.buffer[transmission.index++] = OC_I2C_REG(RXR);
            if (transmission.index < transmission.length) {
                if (transmission.index == (transmission.length - 1)) {
                    OC_I2C_REG(CR) = OC_I2C_RD | OC_I2C_ACK | OC_I2C_STO;
                } else {
                    OC_I2C_REG(CR) = OC_I2C_RD;
                }
            } else {
                transmission.state = I2C_IDLE;
                twi_done();
            }
            break;
        case I2C_DONE:
            transmission.state = I2C_IDLE;
            twi_done();
            break;
    }
}
