/*
 * Timer services
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "timer.h"
#include "mbsoc.h"
#include <assert.h>


static struct timer_svc_data {
    uint64_t    tick;
    list_t      timers;
} timer_data;


/**
 * Timer tick ISR
 */
static void timer_tick (void *data)
{
    list_t *iter, *next;

    timer_data.tick++;

    /*
     * check for expired timers
     * loop can break after first unexpired timer because the list is ordered
     */
    list_for_each_safe(&timer_data.timers, iter, next) {
        timer_t *timer = (timer_t *)iter;
        if (timer->expire == timer_data.tick) {

            // remove from list and add again if periodic
            list_delete(iter);
            if (timer->flags & TIMER_PERIODIC) {
                timer_set(timer, timer->timeout);
            }

            // invoke callback
            timer->func(timer->data);
        } else {
            break;
        }
    }
}

void timer_svc_init (void)
{
    // initialize data
    timer_data.tick = 0;
    list_init_head(&timer_data.timers);

    // install tick isr
    XIOModule_Connect(&xio, XIN_IOMODULE_FIT_1_INTERRUPT_INTR, timer_tick, NULL);
    XIOModule_Enable(&xio, XIN_IOMODULE_FIT_1_INTERRUPT_INTR);
}

void timer_init (timer_t *timer, uint8_t flags, timer_fn func, void *data)
{
    assert(timer && func);
    assert(flags & (TIMER_ONE_SHOT | TIMER_PERIODIC));

    timer->flags = flags;
    timer->expire = 0;
    timer->func = func;
    timer->data = data;
    list_init_head(&timer->link);
}

/**
 * Helper to compare two timer timeouts
 *
 * *MUST* be called in a critical region
 */
static int8_t timer_cmp (timer_t *a, timer_t *b)
{
    uint32_t a_time, b_time;

    a_time = count32_sub(a->expire, timer_data.tick);
    b_time = count32_sub(b->expire, timer_data.tick);

    if (a_time > b_time) {
        return 1;
    } else if (a_time < b_time) {
        return -1;
    } else {
        return 0;
    }
}

void timer_set (timer_t *timer, uint32_t timeout)
{
    list_t *iter;
    CRITICAL_STORE;

    assert(timer);

    // make sure timer isn't already in list
    timer_cancel(timer);

    // timer list shared with ISR
    CRITICAL_START();

    // update timer data (expiry rollover fine)
    timer->timeout = timeout;
    timer->expire = timer_data.tick + timeout;

    // insert at head if list empty
    if (list_is_empty(&timer_data.timers)) {
        list_add(&timer_data.timers, &timer->link);
        CRITICAL_END();
        return;
    }

    // insert timer in list in order of expiry
    list_for_each(&timer_data.timers, iter) {
        if (timer_cmp(timer, (timer_t *)iter) <= 0) {
            list_insert(iter, &timer->link);
            CRITICAL_END();
            return;
        }
    }

    // loop fell through, insert at end of list
    list_insert(&timer_data.timers, &timer->link);

    CRITICAL_END();
}

void timer_cancel (timer_t *timer)
{
    CRITICAL_STORE;
    assert(timer);

    CRITICAL_START();
    list_delete(&timer->link);
    CRITICAL_END();
}

uint32_t timer_get_remaining (timer_t *timer)
{
    uint32_t exp, tick;
    CRITICAL_STORE;

    CRITICAL_START();
    exp = timer->expire;
    tick = timer_data.tick;
    CRITICAL_END();

    return count32_sub(exp, tick);
}

uint32_t timer_get_ticks (void)
{
    uint32_t count;
    CRITICAL_STORE;

    CRITICAL_START();
    count = timer_data.tick;
    CRITICAL_END();

    return count;
}

uint32_t count32_sub (uint32_t a, uint32_t b)
{
    if (a < b) {
        return a + (UINT32_MAX - b);
    } else {
        return a - b;
    }
}

