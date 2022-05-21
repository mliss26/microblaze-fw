/*
 * Utility macros
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include "gcnt.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* some bit macros */
#define BIT_GET(p,b)            ((p) &   (1<<(b)))
#define BIT_SET(p,b)            ((p) |=  (1<<(b)))
#define BIT_CLR(p,b)            ((p) &= ~(1<<(b)))
#define BIT_XOR(p,b)            ((p) ^=  (1<<(b)))
#define BIT_IS_SET(p,b)         (((p) & (1<<(b))) == (1<<(b)))

/* general purpose macros */
#define CONCAT(X,Y)             X ## Y
#define CONCAT_EXP(X,Y)         CONCAT(X,Y)

#define CONTAINER_OF(s,m,p)     (s *)((uint8_t *)(p) - offsetof(s,m))

#define ARRAY_SIZE(a)           (sizeof(a)/sizeof((a)[0]))

#define BMSK_GET_BIT(bm,bit)    (bm[bit/(sizeof(bm[0])*8)] >> (bit%(sizeof(bm[0])*8)) & 0x1)
#define BMSK_SET_BIT(bm,bit,v)  (bm[bit/(sizeof(bm[0])*8)] = v ? \
                                        (bm[bit/(sizeof(bm[0])*8)] | 1<<(bit%(sizeof(bm[0])*8))) : \
                                        (bm[bit/(sizeof(bm[0])*8)] & ~(1<<(bit%(sizeof(bm[0])*8)))))


#define log(fmt, ...)   \
    do { \
        uint64_t tick = gcnt_get(); \
        xil_printf("0x%06x%08x: " fmt "\r\n", (uint32_t)(tick>>32), (uint32_t)tick, ##__VA_ARGS__); \
    } while (0)


#endif /* _UTIL_H_ */
