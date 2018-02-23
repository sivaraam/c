#include <stdlib.h>
#include <stdbool.h>
#include "common.h"
#include "bmp/bmp_helpers.h"
#include "maze_solver_helpers.h"
#include "maze_graph_bridge.h"
#include "bfs_frontier/queue.h"

#ifdef KS_MAZE_SOLVER_DEBUG
#include <stdio.h>
#endif

#define MAX_NEIGHBOURS 4

/**
 * Returns non-zero value if the given pixel in the maze is a hurdle pixel.
 * Else returns 0.
 */
int is_hurdle_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->width*maze->height)
	{
		fprintf(stderr, "is_hurdle_pixel: Invalid pixel %u\n", pixel);
		exit(EXIT_FAILURE);
	}
#endif

	unsigned char *const pixel_byte = maze->data + get_pixel_offset(maze->width, maze->padding, pixel);

	if ((*pixel_byte&HURDLE_PIXEL) == HURDLE_PIXEL) // It's enough to check one byte for now
	{
		return 1;
	}

	return 0;
}

/**
 * Returns non-zero value if the given pixel in the maze is a clear pixel.
 * Else returns 0.
 */
int is_clear_pixel(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	if (pixel >= maze->width*maze->height)
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
	struct openings *const o = malloc(sizeof(struct openings));

	if (o == NULL)
	{
		return NULL;
	}

	long start_gate_pixel = find_gate(maze, 0, maze->width-1),
	     end_gate_pixel = find_gate(maze, (maze->width)*(maze->height-1), maze->width*maze->height-1);

	if (start_gate_pixel == -1 || end_gate_pixel == -1)
	{
		return NULL;
	}

	o->start_gate_pixel = start_gate_pixel;
	o->end_gate_pixel = end_gate_pixel;
	return o;
}

#ifdef KS_MAZE_SOLVER_DEBUG
void print_ascii_maze(struct maze_image *const maze)
{
	for (unsigned pixel=0; pixel<maze->width*maze->height; pixel++)
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
	unsigned clear_pixels=0;

	if (maze == NULL)
	{
		return 1;
	}

	for (unsigned pixel=0; pixel<maze->width*maze->height; pixel++)
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
			fflush(stdout);
#endif

			clear_pixels++;
		}
	}

	maze->clear_pixels = clear_pixels;
	return 0;
}

/**
 * The data structure used to hold the four possible neighbouring clear pixels
 * for a given pixel.
 *
 * The value is non-negative for a valid neighbour.
 */
struct pixel_neighbours
{
	long neighbour[4];

};

/**
 * Identifies valid clear pixel neighbours for the given "start gate" pixel and returns a valid
 * 'struct pixel_neighbours' object.
 */
static
struct pixel_neighbours find_start_gate_neighbours(struct maze_image *const maze, unsigned sg_pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	p = sg_pixel;

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	bottom = p+maze->width;

	struct pixel_neighbours pn = { {-1L, -1L, -1L, -1L} };

#ifdef KS_MAZE_SOLVER_DEBUG
	if (
		p > maze->width*maze->height ||
		bottom > maze->width*maze->height
	)
	{
		fprintf(stderr, "find_start_gate_neighbours: Invalid pixel value\n");
		fprintf(stderr, "find_start_gate_neighbours: p:%ld\t bottom: %ld\n", p, bottom);
		exit(EXIT_FAILURE);
	}
#endif

	if (is_clear_pixel(maze, bottom))
	{
		pn.neighbour[0] = bottom;
	}

	return pn;
}
/**
 * Identifies valid clear pixel neighbours for the given "end gate" pixel and returns a valid
 * 'struct pixel_neighbours' object.
 */
static
struct pixel_neighbours find_end_gate_neighbours(struct maze_image *const maze, unsigned eg_pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	p = eg_pixel;

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	top = p - maze->width;

	struct pixel_neighbours pn = { {-1L, -1L, -1L, -1L} };

#ifdef KS_MAZE_SOLVER_DEBUG
	if (
		p > maze->width*maze->height ||
		top < 0
	)
	{
		fprintf(stderr, "find_end_gate_neighbours: Invalid pixel value\n");
		fprintf(stderr, "find_end_gate_neighbours: p: %ld\t top: %ld\n", p, top);
		exit(EXIT_FAILURE);
	}
#endif

	if (is_clear_pixel(maze, top))
	{
		pn.neighbour[0] = top;
	}

	return pn;
}

/**
 * Identifiies neighbouring clear pixels for the given pixel and returns a valid
 * 'struct pixel_neighbours' object accordingly. Pixels are not expected to be
 * border pixels (those that have fewer than 4 valid neighbours).
 */
static
struct pixel_neighbours find_neighbours(struct maze_image *const maze, unsigned pixel)
{

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	p = pixel;

#ifdef KS_MAZE_SOLVER_DEBUG
	long
#else
	unsigned
#endif
	top = p - maze->width, bottom = p + maze->width,
	left = p - 1, right = p + 1;

	struct pixel_neighbours pn = { {-1L, -1L, -1L, -1L} };


#ifdef KS_MAZE_SOLVER_DEBUG
	if (
		p > maze->width*maze->height ||
		left < 0 || right > maze->width*maze->height ||
		top < 0 || bottom > maze->width*maze->height
	)
	{
		fprintf(stderr, "find_neighbours: Invalid pixel value\n");
		fprintf(stderr, "find_neighbours: p: %ld\t left: %ld\t right: %ld\t "
				"top: %ld\t bottom: %ld\n",
			p, left, right, top, bottom);
		exit(EXIT_FAILURE);
	}
#endif

	if (is_clear_pixel(maze, left))
	{
		pn.neighbour[0] = left;
	}

	if (is_clear_pixel(maze, right))
	{
		pn.neighbour[1] = right;
	}

	if (is_clear_pixel(maze, top))
	{
		pn.neighbour[2] = top;
	}

	if (is_clear_pixel(maze, bottom))
	{
		pn.neighbour[3] = bottom;
	}

	return pn;
}

int initialize_adjacencies(struct maze_image *const maze, struct openings *const o)
{
	for (unsigned clear_pixel=0; clear_pixel<np_list_vals; clear_pixel++)
	{
		const unsigned curr_pixel = (*(np_list+clear_pixel))->pixel;

		struct pixel_neighbours n;

		// special case gates for simplicity
		if (
			curr_pixel != o->start_gate_pixel &&
			curr_pixel != o->end_gate_pixel
		)
		{
			// normal pixel
			n = find_neighbours(maze, curr_pixel);
		}
		else if (curr_pixel == o->start_gate_pixel)
		{
			// start gate_pixel
			n = find_start_gate_neighbours(maze, curr_pixel);
		}
		else
		{
			// end gate pixel
			n = find_end_gate_neighbours(maze, curr_pixel);
		}

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
		printf("initialize_adjacencies: ");
#endif

		for (unsigned curr=0; curr<MAX_NEIGHBOURS; curr++)
		{
			if (n.neighbour[curr] < 0)
			{
				continue;
			}

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
			printf("%ld\t", n.neighbour[curr]);
#endif

			struct node *const adj_node = get_node(n.neighbour[curr]);

#ifdef KS_MAZE_SOLVER_DEBUG
			if (adj_node == NULL)
			{
				fprintf(stderr, "construct_shortest_path: Invalid pixel node.\n");
				exit(EXIT_FAILURE);
			}
#endif

			if (
				add_adjacency((*(np_list+clear_pixel))->pixel_node,
					      adj_node)
			)
			{
				return 1;
			}
		}

#ifdef KS_MAZE_SOLVER_DEBUG_INITIALIZE_ADJACENCIES
		printf("\n initialize_adjacencies: Totally %u adjacencies for pixel: %u\n",
			(*(np_list+clear_pixel))->pixel_node->adjlist.num,
			curr_pixel);
#endif

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
int construct_shortest_path(struct openings *const o, struct sp_queue_head *const sp)
{
	struct node *path_node = get_node(o->end_gate_pixel);

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

		sp_insert_elem(sp, path_elem);

		path_node = path_node->pi;
	}

	// insert the source node
	struct sp_queue_elem *const source_elem = malloc(sizeof(struct sp_queue_elem));

	if (source_elem == NULL)
	{
		return 0;
	}

	source_elem->elem = path_node->pixel;
	sp_insert_elem(sp, source_elem);

	return dest_dist;
}

unsigned find_shortest_path(struct openings *const o, struct sp_queue_head *sp)
{
	struct node *const start_node = get_node(o->start_gate_pixel);

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
	bfsfront_insert_elem(frontier, first);

	while (!bfsfront_queue_empty(frontier))
	{
		struct bfsfront_queue_elem *curr_elem = bfsfront_remove_elem(frontier);

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
					bfsfront_insert_elem(frontier, adj_elem);

					if (curr_adj->pixel == o->end_gate_pixel)
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
	return construct_shortest_path(o, sp);
}

void delete_graph(void)
{
	// initially free the individual nodes
	for (unsigned clear_pixel=0; clear_pixel<np_list_vals; clear_pixel++)
	{
		struct node **const curr_pixel_node = &(*(np_list+clear_pixel))->pixel_node;
		free(*curr_pixel_node);
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
		colour_pixel(maze, curr_elem->elem);
		free(curr_elem);
	}
}
