#include <stddef.h>
#include "queue.h"

void initialise_sp_queue(struct sp_queue_head *head)
{
	head->first = NULL;
	head->last = NULL;
}

int sp_insert_elem(struct sp_queue_head *head, struct sp_queue_elem *elem)
{
	if (head == NULL || elem == NULL)
	{
		return 1;
	}

	elem->next = NULL;
	if (head->last == NULL)
	{
		head->first = elem;
		head->last = elem;
	}
	else
	{
		head->last->next = elem;
		head->last = elem;
	}

	return 0;
}

struct sp_queue_elem *sp_remove_elem(struct sp_queue_head *head)
{
	if (head == NULL)
	{
		return NULL;
	}

	// empty list
	if (head->first == NULL)
	{
		return NULL;
	}

	struct sp_queue_elem *first_elem = head->first;
	head->first = first_elem->next;

	// last node of list
	if (head->last == first_elem)
	{
		head->last = NULL;
	}

	return first_elem;
}

int sp_queue_empty(struct sp_queue_head *head)
{
	if (head == NULL ||
	    (head->first == NULL && head->last == NULL))
	{
		return 1;
	}

	return 0;
}
