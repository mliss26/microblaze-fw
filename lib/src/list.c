/*
 * Doubley linked list implementation
 *
 * Copyright (c) 2022 Matt Liss
 * BSD-3-Clause
 */
#include "list.h"
#include "assert.h"


#ifdef CONFIG_LIST_NULL_CHECKS
#define NULL_CHK(arg)                       \
    assert((arg))
#else
#define NULL_CHK(arg)
#endif


/*
 * Initialize a node as a list head.
 */
void list_init_head (list_t *node)
{
    NULL_CHK(node != NULL);

    node->next = node;
    node->prev = node;
}

/*
 * Return true if the list is empty.
 */
bool list_is_empty (list_t *node)
{
    NULL_CHK(node != NULL);

    return (node->next == node);
}

/*
 * Add a new node after an existing one.
 */
void list_add (list_t *node, list_t *add)
{
    NULL_CHK((node != NULL) && (add != NULL));

    node->next->prev = add;
    add->next = node->next;
    node->next = add;
    add->prev = node;
}

/*
 * Insert a new node before an existing one.
 */
void list_insert (list_t *node, list_t *ins)
{
    NULL_CHK((node != NULL) && (ins != NULL));

    node->prev->next = ins;
    ins->prev = node->prev;
    node->prev = ins;
    ins->next = node;
}

/*
 * Unlink a node from a list.
 */
void list_delete (list_t *node)
{
    NULL_CHK(node != NULL);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

/*
 * Fetch the first node from the list
 */
list_t * list_get_first (list_t *head)
{
    list_t *node;

    NULL_CHK(head != NULL);

    if (list_is_empty(head)) {
        return NULL;
    }
    node = head->next;
    list_delete(node);

    return node;
}

/*
 * Fetch the last node from the list
 */
list_t * list_get_last (list_t *head)
{
    list_t *node;

    NULL_CHK(head != NULL);

    if (list_is_empty(head)) {
        return NULL;
    }
    node = head->prev;
    list_delete(node);

    return node;
}
