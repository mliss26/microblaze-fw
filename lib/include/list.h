/*
 * Doubley linked list
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#ifndef _LIST_H_
#define _LIST_H_


#include "util.h"


// macros to use list as a stack
#define list_push           list_insert
#define list_pop            list_get_last
#define list_top(head)      ((head)->prev)


// macros to use list as a FIFO queue
#define list_enq            list_insert
#define list_deq            list_get_first


/*
 * Doubley linked list node
 */
typedef struct list_t {
    struct list_t *next;
    struct list_t *prev;
} list_t;


/*
 * List initialization macro
 */
#define LIST_INIT_HEAD(list)    {&(list),&(list)}
#define LIST_INITIALIZER        {NULL, NULL}


/*
 * List for each macros
 */
#define list_for_each(list,item)            \
    for((item) = (list)->next; (item) != (list); (item) = (item)->next)
#define list_for_each_safe(list,item,nxt)  \
    for((item) = (list)->next, (nxt) = (item)->next; (item) != (list); (item) = (nxt), (nxt) = (item)->next)


/*
 * List management functions
 */
void list_init_head    (list_t *node);
bool list_is_empty     (list_t *node);
void list_add          (list_t *node, list_t *add);
void list_insert       (list_t *node, list_t *ins);
void list_delete       (list_t *node);
list_t *list_get_first (list_t *head);
list_t *list_get_last  (list_t *head);


#endif
