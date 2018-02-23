#ifndef KS_MAZE_SOLVER_SP_QUEUE
#define KS_MAZE_SOLVER_SP_QUEUE

/**
 * A simple queue implementation that stores the pixel indices that
 * lead to a shortest path from a source to the destination.
 *
 * The first node of the queue is expected to be the sourcee and the last
 * is expected to be the destination.
 */

struct sp_queue_elem
{
	unsigned elem;
	struct sp_queue_elem *next;
};

struct sp_queue_head
{
	struct sp_queue_elem *first;
	struct sp_queue_elem *last;
};

/**
 * Initialise the head of the queue.
 */
void initialise_sp_queue(struct sp_queue_head *head);

/**
 * Insert element at the end of the queue.
 */
void sp_insert_elem(struct sp_queue_head *head, struct sp_queue_elem *elem);

/**
 * Remove the first element from the queue.
 */
struct sp_queue_elem *sp_remove_elem(struct sp_queue_head *head);

/**
 * Returns non-zero value if queue is empty else returns 0.
 */
int sp_queue_empty(struct sp_queue_head *head);

#endif