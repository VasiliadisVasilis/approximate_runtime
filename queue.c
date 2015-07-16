#include <string.h>
#include <stdio.h>
#include "queue.h"
#include <sched.h>

int queue_make_element(element_t **element, void *data)
{
	element_t *el;
	
	el = (element_t*) calloc(1, sizeof(element_t));

	if ( el == NULL )
	{
		return 2;
	}

	el->next = NULL;
	el->data = data;
	
	*element = el;

	return 0;
}

int queue_init(queue_t** queue)
{
	queue_t *q;

	q = (queue_t*) calloc(1, sizeof(queue_t));
	if ( q == NULL )
		return 2;
	
	q->head = (element_t*) calloc(1, sizeof(element_t));
	q->tail = q->head;
	
	if ( q->head == NULL )
	{
		free(q);
		return 2;
	}

	*queue = q;
	return 0;
}

int queue_destroy(queue_t **queue)
{
	element_t *el;

	if ( *queue == NULL )
		return 1;
	
	for ( el = (*queue)->head->next; el != NULL; )
	{
		element_t *n = el->next;
		free(el->data);
		free(el);
		el = n;
	}

	free((*queue)->head);
	free(*queue);

	*queue = NULL;
	
	return 0;
}

int queue_push(queue_t *queue, element_t *element)
{
	element_t *tail;
	do
	{
		tail = queue->tail;

		if ( __sync_bool_compare_and_swap(&(tail->next), NULL, element) )
		{
			if ( __sync_bool_compare_and_swap(&(queue->tail), tail, element) )
			{
				return 0;
			}
		}
		sched_yield();
	}
	while(1);
}

int queue_pop(queue_t *queue, void** data)
{
	element_t *head;
	do
	{
		head = queue->head;

		if (head->next == NULL)
		{
			*data = NULL;
			return 2;
		}
		
		if ( __sync_bool_compare_and_swap(&(queue->head), head, head->next) )
		{
			*data = head->next->data;
			return 0;
		}
		sched_yield();
	}
	while ( 1 );
}

