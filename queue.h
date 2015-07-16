#ifndef _QUEUE_H
#define _QUEUE_H
#include <stdint.h>
#include <stdlib.h>

typedef struct ELEMENT_T
{
	struct ELEMENT_T *next;
	void *data;
} element_t;

typedef struct QUEUE_T
{
	element_t *head, *tail;
	size_t num_elements;
} queue_t;


int queue_init(queue_t** queue);
int queue_pop(queue_t *queue, void** data);
int queue_push(queue_t *queue, element_t *element);
int queue_destroy(queue_t **queue);
int queue_make_element(element_t **element, void *data);

#endif

