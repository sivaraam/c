#include <stdlib.h>
#include <stdbool.h>
#include "common.h"
#include "bmp/bmp_helpers.h"
#include "maze_solver_helpers.h"
#include "maze_graph_bridge.h"
#include "a_star/frontier/pqueue.h"

#if defined KS_MAZE_SOLVER_DEBUG || defined KS_MAZE_SOLVER_DEBUG_FIND_SHORTEST_PATH
#include <stdio.h>
#endif

#ifdef KS_MAZE_SOLVER_DEBUG
/**
 * Returns non-zero value if the given pixel in the maze is a hurdle pixel.
 * Else returns 0.
 */
inline static
int is_hurdle_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->pixels)
	{
		fprintf(stderr, "is_hurdle_pixel: Invalid pixel %u\n", pixel);
		exit(EXIT_FAILURE);
	}
#endif

	const unsigned char *const pixel_byte = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	// It's enough to check one byte for now
	return ((*pixel_byte&HURDLE_PIXEL) == (*pixel_byte)) ? 1 :  0;
}
#endif

inline
int is_clear_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->pixels)
	{
		fprintf(stderr, "is_clear_pixel: Invalid pixel %u\n", pixel);
		exit(EXIT_FAILURE);
	}
#endif

	const unsigned char *const pixel_byte = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	// It's enough to check one byte for now
	return ((*pixel_byte&CLEAR_PIXEL) == CLEAR_PIXEL) ? 1 : 0;
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

unsigned find_shortest_path(struct openings *const gates, struct sp_queue_head *sp,
                            const double *const heuristic_vector)
{
	struct node *const start_node = (*(np_list+gates->start_gate_pixel))->pixel_node;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (start_node == NULL)
	{
		fprintf(stderr, "construct_shortest_path: Invalid pixel node.\n");
		exit(EXIT_FAILURE);
	}

	unsigned nodes_expanded = 0;
#endif

	bool found_dest = false, out_of_mem = false;

	// initial setup
	start_node->colour = GREY;

	// create the frotier queue head
	struct min_heap *const frontier = malloc(sizeof(struct min_heap));

	if (frontier == NULL)
	{
		return 0;
	}

	initialise_min_heap(frontier);

	// insert the start node into the frontier
	struct heap_elem *const first = malloc(sizeof(struct heap_elem));

	if (first == NULL)
	{
		out_of_mem = true;
		goto CLEANUP;
	}

	first->val = start_node;
	first->key = 0;

#ifdef KS_MAZE_SOLVER_DEBUG
	if (min_heap_insert(frontier, first))
	{
		fprintf(stderr, "find_shortest_path: Inserting into the queue failed!\n");
		exit(EXIT_FAILURE);
	}
#else
	min_heap_insert(frontier, first);
#endif

	while (!min_heap_empty(frontier))
	{
		struct heap_elem *const curr_elem = extract_min(frontier);

#ifdef KS_MAZE_SOLVER_DEBUG
		if (curr_elem == NULL)
		{
			fprintf(stderr, "find_shortest_path: Removing from the queue failed!\n");
			exit(EXIT_FAILURE);
		}
#endif

		// stop expanding and just free obtained memory when,
		//
		//   i) destination has been found (or)
		//  ii) memory issue occurs
		//
		if (!(found_dest | out_of_mem)) {

			struct node *const curr = curr_elem->val;

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
					struct heap_elem *const adj_elem = malloc(sizeof(struct heap_elem));

					if (adj_elem == NULL)
					{
						out_of_mem = true;
						break;
					}

					adj_elem->val = curr_adj;
					adj_elem->key = curr_adj->src_dist + *(heuristic_vector+curr_adj->pixel);

#ifdef KS_MAZE_SOLVER_DEBUG_FIND_SHORTEST_PATH
					printf("find_shortest_path: heuristic: %lf key: %lf for pixel: %u\n",
						*(heuristic_vector+curr_adj->pixel), adj_elem->key, curr_adj->pixel);
#endif

#ifdef KS_MAZE_SOLVER_DEBUG
					if (min_heap_insert(frontier, adj_elem))
					{
						fprintf(stderr, "find_shortest_path: Inserting into the BFS queue failed!");
						exit(EXIT_FAILURE);
					}
#else
					min_heap_insert(frontier, adj_elem);
#endif

					if (curr_adj->pixel == gates->end_gate_pixel)
					{
						found_dest = true;
						break;
					}
				}
			}

			curr->colour = BLACK;

#ifdef KS_MAZE_SOLVER_DEBUG
			nodes_expanded++;
#endif
		}

		free(curr_elem);
	}

#ifdef KS_MAZE_SOLVER_DEBUG
	printf("find_shortest_path: Totally expanded %u nodes.\n", nodes_expanded);
#endif

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
