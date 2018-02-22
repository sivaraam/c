#ifndef KS_MAZE_SOLVER_GRAPH_BRIDGE
#define KS_MAZE_SOLVER_GRAPH_BRIDGE

#include "graph/maze_graph.h"

struct node_list
{
	unsigned pixel;
	struct node *pixel_node;
};

/**
 * The head of the node list that holds the (pixel, node) pairs.
 */
extern struct node_list **np_list;

/**
 * Tracks the number of entries in 'np_list'
 */
extern unsigned np_list_vals;


/**
 * Helper function to creates a valid 'struct node' object for the
 * given pixel and initializes its values to the defaults.
 *
 * Returns NULL in case of an error.
 */
struct node *create_node(unsigned pixel);

/**
 * Creates a new 'struct node_list' entry into 'np_list' for the given
 * node and pixel.
 *
 * Returns 0 on success and non-zero value on failure.
 */
int insert_node(unsigned pixel, struct node *n);

/**
 * Remove the coressponding 'struct node_list' entry from the 'np_list'
 * for the given pixel.
 *
 * Returns 0 on success and non-zero value on failure.
 *
 * TODO: Re-implement 'np_list' as a linked list to achieve this.
 */
int remove_node(unsigned pixel);

/**
 * Free the memory taken up by the 'np_list'.
 */
void delete_np_list(void);

/**
 * Gives the pointer to the graph node that represents the given pixel.
 *
 * Returns NULL if there is no node for the given pixel.
 */
struct  node *get_node(unsigned pixel);

/**
 * Adds 'adj_pixel_node' as an adjacency of 'pixel_node'.
 *
 * Returns 0 on success and non-xero value on error.
 */
int add_adjacency(struct node *pixel_node, struct node *adj_pixel_node);

#endif
