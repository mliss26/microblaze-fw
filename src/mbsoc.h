/*
 * MicroBlaze SoC definitions
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _MBSOC_H_
#define _MBSOC_H_

#include "xparameters.h"
#include "xiomodule.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * General 32-bit register accessor
 */
#define reg32(reg)              (*(volatile uint32_t *)reg##_ADDR)

/*
 * Critical region protection: this should disable interrupts
 * to protect OS data structures during modification. It must
 * allow nested calls, which means that interrupts should only
 * be re-enabled when the outer CRITICAL_END() is reached.
 */
#define CRITICAL_STORE          uint32_t msr
#define CRITICAL_START()        msr = mfmsr(); microblaze_disable_interrupts()
#define CRITICAL_END()          mtmsr(msr)

/*
 * MicroBlaze MCS built-in peripherals
 */
#define GPO(ch)     *(volatile uint32_t *)(XPAR_IOMODULE_0_BASEADDR \
                            + ((ch)*XGPO_CHAN_OFFSET) + XGPO_DATA_OFFSET)
#define GPI(ch)     *(volatile uint32_t *)(XPAR_IOMODULE_0_BASEADDR \
                            + ((ch)*XGPI_CHAN_OFFSET) + XGPI_DATA_OFFSET)

/*
 * LEDs are connected to GP output port 1
 */
#define LEDS        (GPO(1))

/*
 * SoC IO module definitions
 */
#define MBSOC_IOMOD_BASE(i)     (XPAR_IOMODULE_0_IO_BASEADDR + i * 0x1000)

#define GCNT_BASE               MBSOC_IOMOD_BASE(0)
#define GCNT_LO_ADDR            (GCNT_BASE + 0)
#define GCNT_HI_ADDR            (GCNT_BASE + 4)
#define GCNT_LO                 reg32(GCNT_LO)
#define GCNT_HI                 reg32(GCNT_HI)

#define PDM_BASE                MBSOC_IOMOD_BASE(1)
#define PDM_EN_ADDR             (PDM_BASE + 0)
#define PDM_EN                  reg32(PDM_ADDR)
#define PDM_DUTY(i)             *(volatile uint32_t *)(PDM_BASE + 4*(i+1))

#define PRNG_BASE               MBSOC_IOMOD_BASE(2)
#define PRNG_SEED_ADDR          (PRNG_BASE + 0)
#define PRNG_SEED               reg32(PRNG_SEED)
#define PRNG_RAND_ADDR          (PRNG_BASE + 4)
#define PRNG_RAND               reg32(PRNG_RAND)

/*
 * Global instance of the XIO module for BSP functions
 */
extern XIOModule xio;


#endif
