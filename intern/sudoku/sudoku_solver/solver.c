/**** DEV ****/
//#define KS_SUDOKU_DEBUG
/**** DEV *****/

#ifdef KS_SUDOKU_DEBUG
#include <stdio.h>
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "sudoku_solver.h"

// functions used only for debug
#ifdef KS_SUDOKU_DEBUG
void print_table_debug(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX])
{
	for (size_t row=0; row<TABLE_ORDER_MAX; row++)
	{
		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			printf("%u\t", sudoku_table[row][col]);
		}
		printf("\n");
	}
}
#endif


/**
  * Type to hold the possible values for a cell (identified by row and column).
  */
struct possible_entries
{
	bool possible[MAX_VALUE+1]; // an index is set to 'true' if the corresponding value is possible. Indexed from 1 to MAX_VALUE.
	unsigned possibilities; // the number of possible values for a cell
};

/**
  * The tail-queue that holds the "naked single" moves to be done.
  */
struct naked_single
{
	size_t row, col;
	STAILQ_ENTRY(naked_single) entries;     /* Tail queue. */
};

/**
 * The head of the tail-queue that holds "naked single" moves.
 */
STAILQ_HEAD(slisthead, naked_single) naked_singles_head = STAILQ_HEAD_INITIALIZER(naked_singles_head);

/**
  * Inserts a "naked single" move (identified by [row, col]) into the tail-queue.
  */
void insert_naked_single(size_t row, size_t col)
{

#ifdef KS_SUDOKU_DEBUG
	if (row >= TABLE_ORDER_MAX || col >= TABLE_ORDER_MAX)
	{
		fprintf(stderr, "insert_naked_single: invalid insertion row: %zu, col: %zu", row, col);
		exit(EXIT_FAILURE);
	}
#endif

	struct naked_single *n1 = malloc(sizeof(struct naked_single));
	n1->row = row;	n1->col = col;
	STAILQ_INSERT_TAIL(&naked_singles_head, n1, entries);
}

/**
  * Returns the "naked single" move for the given cell.
  */
unsigned find_naked_single(struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				size_t row, size_t col)
{
	for (size_t value=MIN_VALUE; value<=MAX_VALUE; value++)
	{
		if (possible_values[row][col].possible[value] == true)
		{
			return value;
		}
	}

#ifdef KS_SUDOKU_DEBUG
	fprintf(stderr, "find_naked_single: invalid request. row: %zu, col: %zu has %u possibilities!\n", row, col, possible_values[row][col].possibilities);
	fprintf(stderr, "find_naked_single: possibility vector: \n");
	for (size_t value=MIN_VALUE; value<=MAX_VALUE; value++)
		fprintf(stderr, "%zu: %d\t", value, possible_values[row][col].possible[value]);
//	print_table_debug(sudoku_table);
	exit(EXIT_FAILURE);
#endif

	return 0; // for safety
}

/**
  * Used to avoid redundancy in the 'update_possibility' function.
  */
void update_possibilities_helper(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				 struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				 size_t row, size_t col, unsigned val,
				 size_t search_row_start, size_t search_row_end,
				 size_t search_col_start, size_t search_col_end)
{
	// search the remaining cells of the square for values which are not possible
	for (size_t search_row=search_row_start; search_row<=search_row_end; search_row++)
	{
		for (size_t search_col=search_col_start; search_col<=search_col_end; search_col++)
		{
			if ((search_row_start == search_row_end && search_col == col) || // ignore the col that caused the update during the row update
			    (search_col_start == search_col_end && search_row == row) || // ignore the row that caused the update during the col update
			    ((search_row_start != search_row_end && search_col_start != search_col_end) && (search_row == row || search_col == col)) // ignore the row and col which have already been updated during a square update
				 )
			{

#ifdef KS_SUDOKU_DEBUG
				printf("update_possibilities_helper: skipping row: %zu, col: %zu\nsearch_row_start: %zu\t search_row_end: %zu\nsearch_col_start: %zu\t search_col_end: %zu\n", search_row, search_col, search_row_start, search_row_end, search_col_start, search_col_end);
#endif

				continue;
			}

			if (sudoku_table[search_row][search_col] == 0 && possible_values[search_row][search_col].possible[val] != false)
			{
				possible_values[search_row][search_col].possible[val] = false;
				possible_values[search_row][search_col].possibilities--;
				if (possible_values[search_row][search_col].possibilities == 1)
				{

#ifdef KS_SUDOKU_DEBUG
					printf("update_possibilities_helper: only_possibility: %u for row: %zu, col: %zu\n", find_naked_single(possible_values, search_row, search_col), search_row, search_col);
#endif

					insert_naked_single(search_row, search_col);
				}

#ifdef KS_SUDOKU_DEBUG
				else if (possible_values[search_row][search_col].possibilities == 0)
				{
					fprintf(stderr, "update_possibilities_helper: incorrect move by updating row: %zu, col: %zu with %u\n", row, col, val);
					fprintf(stderr, "update_possibilities_helper: it left row: %zu, col: %zu with no possibilities\n", search_row, search_col);
					print_table_debug(sudoku_table);
					exit(EXIT_FAILURE);
				}
				else
				{
					printf("update_possibilities_helper: %u possibilities for row: %zu, col: %zu\n", possible_values[search_row][search_col].possibilities, search_row, search_col);
				}
#endif

			}
		}
	}
}

/*
 * Update the possibilities of cells as a consequence of assignment of 'val' to the given cell.
 *
 *  sudoku_table - the table containing the sudoku board
 *  possible_values - lookup table for possible values of different cells in the
 *                    sudoku board.
 *  (row, col) - the cell due to which the update is being performed
 *  val - the value being inserted into (row, col)
 *  (search_row_start, search_row_end) - the range of rows which must be updated
 *  (search_col_start, search_col_end) - the range of colums which must be updated
 */
void update_possibilities(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
			  struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
			  size_t row, size_t col, unsigned val)
{
	// update the cells in the same row
	update_possibilities_helper(sudoku_table, possible_values, row, col, val, row, row, 0, TABLE_ORDER_MAX-1);

	// update the cells in the same column
	update_possibilities_helper(sudoku_table, possible_values, row, col, val, 0, TABLE_ORDER_MAX-1, col, col);

	// find corners of square
	const size_t top_left_row = (row/3)*3;
	const size_t top_left_col = (col/3)*3;

	// update the remaining cell in the square
	update_possibilities_helper(sudoku_table, possible_values, row, col, val, top_left_row, top_left_row+SQUARE_DIMENSION-1, top_left_col, top_left_col+SQUARE_DIMENSION-1);
}

void solve_hidden_singles_helper(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				 struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				 size_t search_row_start, size_t search_row_end,
				 size_t search_col_start, size_t search_col_end,
				 unsigned val)
{
	unsigned occurrence = 0;
	size_t possible_row = 0, possible_col = 0;
	for (size_t search_row=search_row_start; search_row<=search_row_end; search_row++)
	{
		for (size_t search_col=search_col_start; search_col<=search_col_end; search_col++)
		{
			if (sudoku_table[search_row][search_col] == 0)
			{
				if (possible_values[search_row][search_col].possible[val] == true)
				{
					occurrence++;
					possible_row = search_row;
					possible_col = search_col;
				}
			}
		}
	}

	if (occurrence == 1u)
	{

#ifdef KS_SUDOKU_DEBUG
		printf("solve_hidden_singles: found hidden single %u for row: %zu, col: %zu\n", val, possible_row, possible_col);
#endif

		sudoku_table[possible_row][possible_col] = val;
		update_possibilities(sudoku_table, possible_values, possible_row, possible_col, val);
	}
}

/**
  * Identify the "hidden singles" and solve (fill in) them.
  * This might help in identifying other possibilities such as "naked singles",
  * "naked doubles" or even other "hidden singles".
  *
  * This is to be done by searching rows for values which exist in only one cell and
  * confirming whether they are "single" by checking their other nighbours.
 */
void solve_hidden_singles(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
			 struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX])
{
	for (unsigned val=MIN_VALUE; val<=MAX_VALUE; val++)
	{

		// search for hidden singles in rows

#ifdef KS_SUDOKU_DEBUG
		printf("solve_naked_singles: searching for hidden singles in rows for val: %u\n\n", val);
#endif

		for (size_t row=0; row<TABLE_ORDER_MAX; row++)
		{
			solve_hidden_singles_helper(sudoku_table, possible_values, row, row, 0, TABLE_ORDER_MAX-1, val);
		}

		// search for hidden singles in cols

#ifdef KS_SUDOKU_DEBUG
		printf("solve_naked_singles: searching for hidden singles in cols for val: %u\n\n", val);
#endif

		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			solve_hidden_singles_helper(sudoku_table, possible_values, 0, TABLE_ORDER_MAX-1, col, col, val);
		}

		// search for hidden singles in squares

#ifdef KS_SUDOKU_DEBUG
		printf("solve_naked_singles: searching for hidden singles in squares for val: %u\n\n", val);
#endif

		for (size_t row=0; row<TABLE_ORDER_MAX; row+=SQUARE_DIMENSION)
		{
			for (size_t col=0; col<TABLE_ORDER_MAX; col+=SQUARE_DIMENSION)
			{
				solve_hidden_singles_helper(sudoku_table, possible_values, row, row+SQUARE_DIMENSION-1, col, col+SQUARE_DIMENSION-1, val);
			}
		}
	}
}

/**
  * Solve (fill in) the "naked single" possibilities in the sudoku table.
  * The 'possible_entries' table should be initialized for the given 'sudoku_table'.
  */
void solve_naked_singles(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
			 struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX])
{
	while (!STAILQ_EMPTY(&naked_singles_head))
	{
		struct naked_single *curr = STAILQ_FIRST(&naked_singles_head);
		STAILQ_REMOVE_HEAD(&naked_singles_head, entries);

		unsigned naked_single = find_naked_single(possible_values, curr->row, curr->col);
		sudoku_table[curr->row][curr->col] = naked_single;
		possible_values[curr->row][curr->col].possibilities--; // not of much use as it zeros the result (possibly helpful for debugging)

#ifdef KS_SUDOKU_DEBUG
		printf("solve_naked_singles: only possibility %u for row: %zu, col: %zu\n", naked_single, curr->row, curr->col);
		update_possibilities(sudoku_table, possible_values, curr->row, curr->col, naked_single);
		struct naked_single *print_curr = STAILQ_FIRST(&naked_singles_head);
		printf("solve_naked_singles: Naked single possibilities:\n");
		while (print_curr != NULL)
		{
			printf("%zu\t%zu\n", print_curr->row, print_curr->col);
			print_curr = STAILQ_NEXT(print_curr, entries);
		}
		printf("\n\n");
#endif

		free(curr);
	}
}

void solve(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
	   struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX])
{
	solve_naked_singles(sudoku_table, possible_values);

#ifdef KS_SUDOKU_DEBUG
	printf("solve: possibility vectors after initally trying to solve 'naked_singles':\n");
	for (size_t row=0; row<TABLE_ORDER_MAX; row++)
	{
		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			if (sudoku_table[row][col] == 0)
			{
				printf("solve_sudoku: %u possible values for row: %zu, col: %zu\n", possible_values[row][col].possibilities, row, col);
				for (size_t value=MIN_VALUE; value<=MAX_VALUE; value++)
				{
					if (possible_values[row][col].possible[value] == true)
					{
						printf("%zu\t", value);
					}
				}
				printf("\n\n");
			}
		}
	}
#endif

	solve_hidden_singles(sudoku_table, possible_values);

#ifdef KS_SUDOKU_DEBUG
	printf("solve: possibility vectors after initally trying to solve 'hidden singles':\n");
	for (size_t row=0; row<TABLE_ORDER_MAX; row++)
	{
		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			if (sudoku_table[row][col] == 0)
			{
				printf("solve_sudoku: %u possible values for row: %zu, col: %zu\n", possible_values[row][col].possibilities, row, col);
				for (size_t value=MIN_VALUE; value<=MAX_VALUE; value++)
				{
					if (possible_values[row][col].possible[value] == true)
					{
						printf("%zu\t", value);
					}
				}
				printf("\n\n");
			}
		}
	}
#endif

}

/**
  * Used to avoid redundancy in the 'initialise_possible_values' function.
  */
void initialise_possible_values_helper(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				       struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				       size_t row, size_t col,
				       size_t search_row_start, size_t search_row_end,
				       size_t search_col_start, size_t search_col_end)
{
	for (size_t search_row=search_row_start; search_row<=search_row_end; search_row++)
	{
		for (size_t search_col=search_col_start; search_col<=search_col_end; search_col++)
		{
			if ((search_row_start == search_row_end && search_col == col) || // ignore the col that caused the update during the row update
			    (search_col_start == search_col_end && search_row == row) || // ignore the row that caused the update during the col update
			    ((search_row_start != search_row_end && search_col_start != search_col_end) && (search_row == row || search_col == col)) // ignore the row and col which have already been updated during a square update
				 )
			{
				continue;
			}

			const unsigned val = sudoku_table[search_row][search_col];
			if (val != 0)
			{
				if (possible_values[row][col].possible[val] != false) // avoid multiple counting
				{
					possible_values[row][col].possible[val] = false;
					possible_values[row][col].possibilities--;
				}
			}
		}
	}
}

/**
  * Initialise 'possible_entries' with values that are possible for a given sudoku cell.
  * Obviously the cell is expected not to be an already filled one.
  *
  * Initialization is done by identifying values that are not possible for the given square
  * by linearly searching the corresponding row, column and the square. The 'possible_entries'
  * is updated accordingly.
  */
void initialise_possible_values(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX],
				size_t row, size_t col)
{

#ifdef KS_SUDOKU_DEBUG
	if (sudoku_table[row][col] != 0)
	{
		fprintf(stderr, "initialise_possible_values: Trying to initialise an already filled cell\n");
		exit(EXIT_FAILURE);
	}
#endif

	// initially initialise all possibilities to 'true'
	for (size_t val=MIN_VALUE; val<=MAX_VALUE; val++)
	{
		possible_values[row][col].possible[val] = true;
	}
	possible_values[row][col].possibilities = NUMBER_OF_VALUES;

	// initialise possible_values of cell by searching values in the same row
	initialise_possible_values_helper(sudoku_table, possible_values, row, col, row, row, 0, TABLE_ORDER_MAX-1);

	// intialise possible_values of cell by searching values in the same column
	initialise_possible_values_helper(sudoku_table, possible_values, row, col, 0, TABLE_ORDER_MAX-1, col, col);

	// find corners of square
	const size_t top_left_row = (row/3)*3;
	const size_t top_left_col = (col/3)*3;

	// intialise possible_values of cell by searching values in the same square
	initialise_possible_values_helper(sudoku_table, possible_values, row, col, top_left_row, top_left_row+SQUARE_DIMENSION-1, top_left_col, top_left_col+SQUARE_DIMENSION-1);
}

void solve_sudoku(unsigned sudoku_table[TABLE_ORDER_MAX][TABLE_ORDER_MAX])
{
	// the lookup table used to identify the possibilities of different cells
	struct possible_entries possible_values[TABLE_ORDER_MAX][TABLE_ORDER_MAX];

	STAILQ_INIT(&naked_singles_head);

#ifdef KS_SUDOKU_DEBUG
	printf("\n");
	printf("solve_sudoku: SUDOKU table obtained as input\n");
	for (size_t row=0; row<TABLE_ORDER_MAX; row++)
	{
		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			printf("%u\t", sudoku_table[row][col]);
		}
		printf("\n\n");
	}
#endif

	for (size_t row=0; row<TABLE_ORDER_MAX; row++)
	{
		for (size_t col=0; col<TABLE_ORDER_MAX; col++)
		{
			if (sudoku_table[row][col] == 0)
			{
				// initialise the possible values
				initialise_possible_values(sudoku_table, possible_values, row, col);

#ifdef KS_SUDOKU_DEBUG
				printf("solve_sudoku: %u possible values for row: %zu, col: %zu\n", possible_values[row][col].possibilities, row, col);
				for (size_t value=MIN_VALUE; value<=MAX_VALUE; value++)
				{
					if (possible_values[row][col].possible[value] == true)
					{
						printf("%zu\t", value);
					}
				}
				printf("\n\n");
#endif

				// This could also be done in 'initialise_possible_values_helper'.
				// But doing this here saves us some unwanted checking.
				if (possible_values[row][col].possibilities == 1)
				{
					insert_naked_single(row, col);
				}
			}
		}
	}

#ifdef KS_SUDOKU_DEBUG
	struct naked_single *curr = STAILQ_FIRST(&naked_singles_head);
	printf("solve_sudoku: Naked single possibilities:\n");
	while (curr != NULL)
	{
		printf("%zu\t%zu\n", curr->row, curr->col);
		curr = STAILQ_NEXT(curr, entries);
	}
	printf("\n\n");
#endif

	solve(sudoku_table, possible_values);
}
