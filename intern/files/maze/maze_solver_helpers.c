#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "common.h"
#include "bmp/bmp_helpers.h"
#include "maze_solver_helpers.h"
#include "maze_graph_bridge.h"
#include "bfs_frontier/queue.h"

#ifdef KS_MAZE_SOLVER_DEBUG
#include <stdio.h>
#endif

#ifdef KS_MAZE_SOLVER_DEBUG
/**
 * Returns non-zero value if the given pixel in the maze is a hurdle pixel.
 * Else returns 0.
 */
static
int is_hurdle_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->pixels)
	{
		fprintf(stderr, "is_hurdle_pixel: Invalid pixel %u\n", pixel);
		exit(EXIT_FAILURE);
	}
#endif

	unsigned char *const pixel_byte = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	if ((*pixel_byte&HURDLE_PIXEL) == (*pixel_byte)) // It's enough to check one byte for now
	{
		return 1;
	}

	return 0;
}
#endif

/**
 * Returns non-zero value if the given pixel in the maze is a clear pixel.
 * Else returns 0.
 */
static
int is_clear_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->pixels)
	{
		fprintf(stderr, "is_clear_pixel: Invalid pixel %u\n", pixel);
		exit(EXIT_FAILURE);
	}
#endif

	unsigned char *const pixel_byte = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	if ((*pixel_byte&CLEAR_PIXEL) == CLEAR_PIXEL) // It's enough to check one byte for now
	{
		return 1;
	}

	return 0;
}

/**
 * Finds a 'gate' in the given range. A gate is the only clear pixel in the given range.
 *
 * On success, returns the value of the pixel of the gate cast to a long.
 * If one is not found returns -1;
 */
static
long find_gate(struct maze_image *const maze, unsigned start_pixel, unsigned end_pixel)
{
	long gate = -1;

#ifdef KS_MAZE_SOLVER_DEBUG_FIND_GATE
	bool found_gate = false;
#endif

	// find the pixel of the start gate
	for (unsigned pixel = start_pixel; pixel<=end_pixel; pixel++)
	{
		unsigned char *const pixel_ptr = maze->data+get_pixel_offset(maze->width, maze->padding, pixel);

		if ((*pixel_ptr&CLEAR_PIXEL) == CLEAR_PIXEL)
		{

#ifdef KS_MAZE_SOLVER_DEBUG_FIND_GATE
			if (found_gate)
			{
				fprintf(stderr, "find_gate: Two start gates found!\n");
				exit(EXIT_FAILURE);
			}
#endif

			gate = pixel;

#ifdef KS_MAZE_SOLVER_DEBUG_FIND_GATE
			found_gate = true;
#else
			break;
#endif

		}

	}

	return gate;
}

struct openings *find_openings(struct maze_image *const maze)
{
	struct openings *const gates = malloc(sizeof(struct openings));

	if (gates == NULL)
	{
		return NULL;
	}

	long start_gate_pixel = find_gate(maze, 0, maze->width-1),
	     end_gate_pixel = find_gate(maze, (maze->width)*(maze->height-1), maze->pixels-1);

	if (start_gate_pixel == -1 || end_gate_pixel == -1)
	{
		return NULL;
	}

	gates->start_gate_pixel = start_gate_pixel;
	gates->end_gate_pixel = end_gate_pixel;
	return gates;
}

#ifdef KS_MAZE_SOLVER_DEBUG
void print_ascii_maze(struct maze_image *const maze)
{
	for (unsigned pixel=0; pixel<maze->pixels; pixel++)
	{
		if (pixel%maze->width == 0)
		{
			printf("\n");
		}
		
		if (is_clear_pixel(maze, pixel))
		{
			printf(" ");
		}
		else if (is_hurdle_pixel(maze, pixel))
		{
			printf("\u2588"); // unicode block character
		}
		else
		{
			fprintf(stderr, "Unexpected Hex!!\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("\n");
}
#endif

int create_graph(struct maze_image *const maze)
{
#ifdef KS_MAZE_SOLVER_DEBUG
	unsigned clear_pixels=0;
#endif

	if (maze == NULL)
	{
		return 1;
	}

	// initialise the 'np_list'
	if (initialize_np_list(maze->pixels))
	{
		// delete the partial np list
		delete_np_list();
		return 1;
	}

	for (unsigned pixel=0; pixel<maze->pixels; pixel++)
	{
		if (is_clear_pixel(maze, pixel))
		{
			// create the node
			struct node *const n = create_node(pixel);

			if (n == NULL)
			{

#ifdef KS_MAZE_SOLVER_DEBUG_CREATE_GRAPH
				fprintf(stderr, "create_graph: 'create_node' failed for pixel: %u\n", pixel);
#endif

				return 1;
			}

			// insert the node into the 'np_list'
			if (insert_node(pixel, n))
			{

#ifdef KS_MAZE_SOLVER_DEBUG_CREATE_GRAPH
				fprintf(stderr, "create_graph: 'insert_node' failed for pixel: %u\n", pixel);
#endif

				return 1;
			}

#ifdef KS_MAZE_SOLVER_DEBUG_CREATE_GRAPH
			printf("create_graph: node value: %u\n", n->pixel);
#endif

#ifdef KS_MAZE_SOLVER_DEBUG
			clear_pixels++;
#endif

		}
	}

#ifdef KS_MAZE_SOLVER_DEBUG
	printf("create_graph: Totally found %u clear pixels\n", clear_pixels);
#endif

	return 0;
}

int initialize_adjacencies(struct maze_image *const maze, struct openings *const gates)
{
	// start searching from the second pixel of the second row
	// and initialize the adjacencies by checking the left and top pixels alone
	for (unsigned height = 1; height < maze->height-1; height++)
	{
		for (unsigned width=1; width < maze->width - 1; width++)
		{
			const unsigned pixel = (maze->width) * height + width;

			if (is_clear_pixel(maze, pixel))
			{
				const unsigned left = pixel - 1,
				               top  = pixel - maze->width;

				if (is_clear_pixel(maze, left))
				{

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
					printf("initialize_adjacencies: Found adjacencies %u and %u\n",
					       pixel, left);
#endif

					if (
						add_adjacency((*(np_list+pixel))->pixel_node,
						              (*(np_list+left))->pixel_node)
					)
					{
						return 1;
					}
				}

				if (is_clear_pixel(maze, top))
				{

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
					printf("initialize_adjacencies: Found adjacencies %u and %u\n",
					       pixel, top);
#endif

					if (
						add_adjacency((*(np_list+pixel))->pixel_node,
						              (*(np_list+top))->pixel_node)
					)
					{
						return 1;
					}
				}
			}
		}
	}

	// Add adjacencies for the end pixel
	// This depends on the observation that the pixel below the start node
	// and the pixel above the end node should be adjacencies for sure.

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
	if (is_clear_pixel(maze, gates->end_gate_pixel - maze->width))
	{
		printf("initialize_adjacencies: Found adjacencies %u and %u\n",
		       gates->end_gate_pixel, gates->end_gate_pixel - maze->width);
	}
	else
	{
		fprintf(stderr, "initialize_adjacencies: Could not find adjacency for end gate pixel!\n");
		exit(EXIT_FAILURE);
	}
#endif

	if (
		add_adjacency((*(np_list + gates->end_gate_pixel))->pixel_node,
		              (*(np_list + gates->end_gate_pixel - maze->width))->pixel_node)
	)
	{
		return 1;
	}

	return 0;
}

int find_deadend_nodes(struct maze_image *maze, struct openings *gates, struct de_queue_head *de_nodes)
{
	unsigned initial_dead_end_nodes = 0;

	for (unsigned pixel=0; pixel<maze->pixels; pixel++)
	{
		if (pixel == gates->start_gate_pixel ||
		    pixel == gates->end_gate_pixel)
		{
			continue;
		}

		if (is_clear_pixel(maze, pixel) && (*(np_list + pixel))->pixel_node->adjlist.num == 1)
		{
			struct de_queue_elem *de_node = malloc(sizeof(struct de_queue_elem));
			if (de_node == NULL)
			{
				return ERRMEMORY;
			}
			de_node->elem = (*(np_list+pixel))->pixel_node;

#ifdef KS_MAZE_SOLVER_DEBUG
			if (de_queue_insert_elem(de_nodes, de_node))
			{
				fprintf(stderr, "find_deadend_nodes: Inserting into the dead end queue failed!\n");
				exit(EXIT_FAILURE);
			}
#else
			de_queue_insert_elem(de_nodes, de_node);
#endif

			initial_dead_end_nodes++;
		}
	}

#ifdef KS_MAZE_SOLVER_DEBUG
	if (initial_dead_end_nodes > INT_MAX)
	{
		fprintf(stderr, "find_deadend_nodes: initial_dead_end_nodes doesn't fit into an 'int'\n");
		exit(EXIT_FAILURE);
	}
#endif

	return initial_dead_end_nodes;
}

int prune_deadend_nodes(struct de_queue_head *de_nodes, unsigned initial_deadend_nodes)
{

	unsigned pruned_nodes = 0;
	unsigned second_deadend_nodes = 0;
	bool cleanup = false;

	while (!de_queue_empty(de_nodes))
	{
		struct de_queue_elem *curr_dead_node_elem = de_queue_remove_elem(de_nodes);

		if (cleanup == false)
		{
#ifdef KS_MAZE_SOLVER_DEBUG
			if (curr_dead_node_elem == NULL)
			{
				fprintf(stderr, "prune_deadend_nodes: Removing element from dead end queue failed!\n");
				exit(EXIT_FAILURE);
			}
#ifdef KS_MAZE_SOLVER_DEBUG_PRUNE_DEADEND_NODES
			else
			{
				printf("prune_deadend_nodes: Pruning node %u (%p) with %u adjacencies.\n",
				       curr_dead_node_elem->elem->pixel, (void *) curr_dead_node_elem->elem,
				       curr_dead_node_elem->elem->adjlist.num);
			}
#endif
#endif

			if (curr_dead_node_elem->elem->adjlist.num == 0)
			{
				// the node was already pruned as a consequence of its closed neighbours
				free(curr_dead_node_elem);
				pruned_nodes++;
				continue;
			}
			struct node *only_adj = *(curr_dead_node_elem->elem->adjlist).adjs;

#ifdef KS_MAZE_SOLVER_DEBUG
			int remove_adj_ret_val = remove_adjacency(only_adj, curr_dead_node_elem->elem);
			if (remove_adj_ret_val == ERRNOADJ)
			{
				fprintf(stderr, "prune_deadend_nodes: %u (%p) is not and adjacency of %u (%p) (ERRNOADJ)!\n",
				                 curr_dead_node_elem->elem->pixel, (void *) curr_dead_node_elem->elem,
				                 only_adj->pixel, (void *) only_adj);
				exit(EXIT_FAILURE);
			}
			if (remove_adj_ret_val == ERRMEMORY)
			{
				fprintf(stderr, "prune_deadend_nodes: removing adjacency failed: memory issue (ERROOM)\n");
				exit(EXIT_FAILURE);
			}
#else
			remove_adjacency(only_adj, curr_dead_node_elem->elem);
#endif

			if (only_adj->adjlist.num == 1)
			{
				struct de_queue_elem *only_adj_elem = malloc(sizeof(struct de_queue_elem));
				if (only_adj_elem == NULL)
				{
					return ERRMEMORY;
				}
				only_adj_elem->elem = only_adj;

#ifdef KS_MAZE_SOLVER_DEBUG
				if (de_queue_insert_elem(de_nodes, only_adj_elem))
				{
					fprintf(stderr, "prune_deadend_nodes: Inserting element into dead end queue failed!\n");
					exit(EXIT_FAILURE);
				}
#else
				de_queue_insert_elem(de_nodes, only_adj_elem);
#endif
			}

		}

		free(curr_dead_node_elem);
		pruned_nodes++;

		if (pruned_nodes <= initial_deadend_nodes)
		{
			second_deadend_nodes++;
		}
		if (pruned_nodes <= second_deadend_nodes + initial_deadend_nodes)
		{
			cleanup = true;
		}
	}

#ifdef KS_MAZE_SOLVER_DEBUG
	printf("prune_deadend_nodes: Totally pruned %u nodes from the search space.\n", pruned_nodes);
#endif

	return 0;
}

/**
 * Construct the shortest path from the values of the predecessor of each node
 * starting from the end node.
 *
 * Returns the distance of the end node from the start node on success and 0 in case
 * of an error.
 */
static
int construct_shortest_path(struct openings *const gates, struct sp_queue_head *const sp)
{
	struct node *path_node = (*(np_list+gates->end_gate_pixel))->pixel_node;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (path_node == NULL)
	{
		fprintf(stderr, "construct_shortest_path: Invalid pixel node.\n");
		exit(EXIT_FAILURE);
	}
#endif

	const unsigned dest_dist = path_node->src_dist;
#ifdef KS_MAZE_SOLVER_DEBUG
	printf("construct_shortest_path: Destination is %u pixels away from the source.\n", dest_dist);
#endif

	// sanity check
	if (sp == NULL)
	{
		return 0;
	}

	while (path_node->pi != NULL)
	{
		// insert the current path node
		struct sp_queue_elem *const path_elem = malloc(sizeof(struct sp_queue_elem));

		if (path_elem == NULL)
		{
			return 0;
		}

		path_elem->elem = path_node->pixel;

#ifdef KS_MAZE_SOLVER_DEBUG
		if (sp_insert_elem(sp, path_elem))
		{
			fprintf(stderr, "construct_shortest_path: Inserting %u into shortest path queue failed!", path_elem->elem);
			exit(EXIT_FAILURE);
		}
#else
		sp_insert_elem(sp, path_elem);
#endif

		path_node = path_node->pi;
	}

	// insert the source node
	struct sp_queue_elem *const source_elem = malloc(sizeof(struct sp_queue_elem));

	if (source_elem == NULL)
	{
		return 0;
	}

	source_elem->elem = path_node->pixel;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (sp_insert_elem(sp, source_elem))
	{
		fprintf(stderr, "construct_shortest_path: Inserting into shortest path queue failed!");
		exit(EXIT_FAILURE);
	}
#else
	sp_insert_elem(sp, source_elem);
#endif

	return dest_dist;
}

unsigned find_shortest_path(struct openings *const gates, struct sp_queue_head *sp)
{
	struct node *const start_node = (*(np_list+gates->start_gate_pixel))->pixel_node;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (start_node == NULL)
	{
		fprintf(stderr, "construct_shortest_path: Invalid pixel node.\n");
		exit(EXIT_FAILURE);
	}
#endif

	bool found_dest = false, out_of_mem = false;

	// initial setup
	start_node->colour = GREY;

	// create the frotier queue head
	struct bfsfront_queue_head *const frontier = malloc(sizeof(struct bfsfront_queue_head));

	if (frontier == NULL)
	{
		return 0;
	}

	initialise_bfsfront_queue(frontier);

	// insert the start node into the frontier
	struct bfsfront_queue_elem *const first = malloc(sizeof(struct bfsfront_queue_elem));

	if (first == NULL)
	{
		out_of_mem = true;
		goto CLEANUP;
	}

	first->elem = start_node;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (bfsfront_insert_elem(frontier, first))
	{
		fprintf(stderr, "find_shortest_path: Inserting into the BFS queue failed!\n");
		exit(EXIT_FAILURE);
	}
#else
	bfsfront_insert_elem(frontier, first);
#endif

	while (!bfsfront_queue_empty(frontier))
	{
		struct bfsfront_queue_elem *curr_elem = bfsfront_remove_elem(frontier);

#ifdef KS_MAZE_SOLVER_DEBUG
		if (curr_elem == NULL)
		{
			fprintf(stderr, "find_shortest_path: Removing from the BFS queue failed!\n");
			exit(EXIT_FAILURE);
		}
#endif

		// stop expanding and just free obtained memory when,
		//
		//   i) destination has been found (or)
		//  ii) memory issue occurs
		//
		if (!(found_dest | out_of_mem)) {

			struct node *const curr = curr_elem->elem;

			for (unsigned adj=0; adj<curr->adjlist.num; adj++)
			{
				struct node *curr_adj = *(curr->adjlist.adjs + adj);

				if (curr_adj->colour == WHITE)
				{
					// set the attributes
					curr_adj->colour = GREY;
					curr_adj->src_dist = curr->src_dist+1;
					curr_adj->pi = curr;

					// insert the element into the frontier
					struct bfsfront_queue_elem *const adj_elem = malloc(sizeof(struct bfsfront_queue_elem));

					if (adj_elem == NULL)
					{
						out_of_mem = true;
						break;
					}

					adj_elem->elem = curr_adj;

#ifdef KS_MAZE_SOLVER_DEBUG
					if (bfsfront_insert_elem(frontier, adj_elem))
					{
						fprintf(stderr, "find_shortest_path: Inserting into the BFS queue failed!");
						exit(EXIT_FAILURE);
					}
#else
					bfsfront_insert_elem(frontier, adj_elem);
#endif

					if (curr_adj->pixel == gates->end_gate_pixel)
					{
						found_dest = true;
						break;
					}
				}
			}

			curr->colour = BLACK;
		}

		free(curr_elem);
	}

CLEANUP:
	// free the queue head
	free(frontier);

	if (out_of_mem)
	{
		return 0;
	}

	// construct the shortest path from the values of the predecessors
	return construct_shortest_path(gates, sp);
}

void delete_graph(struct maze_image *maze)
{
	if (np_list == NULL)
	{
		return;
	}

	// initially free the individual nodes
	for (unsigned pixel=0; pixel<maze->pixels; pixel++)
	{
		if (is_clear_pixel(maze, pixel))
		{
			struct node *const curr_pixel_node = (*(np_list+pixel))->pixel_node;
			free(curr_pixel_node->adjlist.adjs);
			free(curr_pixel_node);
		}
	}

	// now delete the np_list itself
	delete_np_list();
}

/**
 * Colour the given pixel in the maze.
 */
inline static
void colour_pixel(struct maze_image *const maze, unsigned pixel)
{
	unsigned char *const pixel_bytes = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	// ordering of RGB for little-endian architecture
	static const unsigned char colour_bytes[3] = {
	   0x00, // blue,
	   0x00, // green
	   0xFF //red
	};

	for (unsigned byte=0; byte<PIXEL_BYTES; byte++)
	{
		*(pixel_bytes + byte) = colour_bytes[byte];
	}
}

void colour_path(struct maze_image *const maze, struct sp_queue_head *const sp)
{
	while (!sp_queue_empty(sp))
	{
		struct sp_queue_elem *const curr_elem = sp_remove_elem(sp);

#ifdef KS_MAZE_SOLVER_DEBUG
		if (curr_elem == NULL)
		{
			fprintf(stderr, "colour_pixel: Removal fron the shortest path queue failed!\n");
			exit(EXIT_FAILURE);
		}
#endif

		colour_pixel(maze, curr_elem->elem);
		free(curr_elem);
	}
}
