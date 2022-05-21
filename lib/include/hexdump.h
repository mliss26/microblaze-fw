/*
 * Hexdump functions for debugging
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _HEXDUMP_H_
#define _HEXDUMP_H_

#include <stddef.h>


#define HEXDUMP_LINE_BYTES          16


void hexdump32(void const *addr, size_t len);


#endif /* _HEXDUMP_H_ */
