#ifndef KS_MAZE_SOLVER_HELPER
#define KS_MAZE_SOLVER_HELPER

#include "maze_solver.h"
#include "shortest_path/queue.h"

/**
 * For the given maze find the start and end gates.
 *
 * Returns a valid 'struct openings' objct on success.
 * In case of failure returns NULL.
 */
struct openings *find_openings(struct maze_image *const maze);

#ifdef KS_MAZE_SOLVER_DEBUG
/**
 * Print an ASCII art version of the given maze.
 */
void print_ascii_maze(struct maze_image *const maze);
#endif

//
// FIXME: Think about moving these to maze_graph_bridge.
//

/**
 * Create the graph nodes for each clear pixel in the maze image and initialise
 * the adjacency list and heuristic values for the node.
 *
 * Returns 0 on success and non-zero value on error (mostly memory error).
 */
int create_graph(struct maze_image *const maze, struct openings *const gates);

/**
 * Find the shortest path from the start gate node to the end gate node
 * in the graph using the heuritic value which estimates the approxiamate
 * cost to the destination from any clear pixel in the maze.
 *
 * Stores the shortest path in the given queue. The queue is expected to be
 * a pointer to a valid queue head which has been initialized.
 *
 * Returns the non-negative distance of the destination from the source on
 * success or 0 in case of failure.
 *
 * Note: In case of failure the queue might be partially filled. Freeing
 * the elements in the queue (and of course the queue head) is the
 * responsibility of the caller.
 */
unsigned find_shortest_path(struct openings *const gates, struct sp_queue_head *const sp);

/**
 * Free up the memory taken up by the graph and its related structure.
 */
void delete_graph(void);

//
// END FIXME
//

/**
 * Colour the pixels corresponding to the shortest path with a colour other
 * than black and white. Expects a pointer to a valid 'struct sp_queue_head'
 * object.
 */
void colour_path(struct maze_image *const maze, struct sp_queue_head *const sp);

#endif
