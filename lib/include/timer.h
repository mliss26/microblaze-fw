/*
 * Timer services
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include "util.h"
#include "list.h"


/**
 * Timer config
 */
#ifndef TIMER_MS_PER_TICK
#define TIMER_MS_PER_TICK       1
#endif
#define TIMER_TICKS_PER_SEC     (1000 / TIMER_MS_PER_TICK)

#define TIMEOUT_IN_MS(t)        ((t) / TIMER_MS_PER_TICK)
#define TIMEOUT_IN_SEC(t)       ((t) * TIMER_TICKS_PER_SEC)

#define TICKS_TO_MS(t)          ((t) * TIMER_MS_PER_TICK)
#define TICKS_TO_SEC(t)         ((t) / TIMER_TICKS_PER_SEC)

/**
 * Timer flags
 */
#define TIMER_ONE_SHOT          0x01
#define TIMER_PERIODIC          0x02


/**
 * Timer callback function
 */
typedef void (* timer_fn) (void *data);


/**
 * Timer structure
 */
typedef struct timer_t {
    list_t      link; // must be first entry
    uint8_t     flags;
    uint32_t    timeout;
    uint32_t    expire;
    timer_fn    func;
    void        *data;
} timer_t;


/**
 * Initialize timer services
 */
void timer_svc_init (void);

/**
 * Initialize a timer
 */
void timer_init (timer_t *timer, uint8_t flags, timer_fn func, void *data);

/**
 * Schedule a timer
 */
void timer_set (timer_t *timer, uint32_t timeout);

/**
 * Cancel a scheduled timer
 */
void timer_cancel (timer_t *timer);

/**
 * Get remaining time in ticks for an active timer
 */
uint32_t timer_get_remaining (timer_t *timer);

/**
 * Query the current tick counter
 */
uint32_t timer_get_ticks(void);

/**
 * Subtract two counter values
 *
 * Performs (a - b) assuming a is in the future from b and both
 * counters may rollover.
 */
uint32_t count32_sub (uint32_t a, uint32_t b);


#endif // _TIMER_H_
