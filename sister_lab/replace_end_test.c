#include <stdio.h>
#include <stdlib.h>
#include "string/find_and_replace.h"

#define MAX_INPUT_SIZE (100+1)
#define MAX_SEARCH_TERM_SIZE (15+1)
#define MAX_REPLACE_TERM_SIZE (15+1)

int main(void) {
	char *input_str = malloc(MAX_INPUT_SIZE), *result = malloc(MAX_INPUT_SIZE);
	char *search = malloc(MAX_SEARCH_TERM_SIZE);
	char *replace = malloc(MAX_REPLACE_TERM_SIZE);

	// get the inputs
	printf("Enter the string: ");
	fgets(input_str, MAX_INPUT_SIZE, stdin);

	printf("Enter search string: ");
	fgets(search, MAX_SEARCH_TERM_SIZE, stdin);

	printf("Enter replace string: ");
//	scanf("%ms", &replace);	
	fgets(replace, MAX_REPLACE_TERM_SIZE, stdin);

//	printf("%s %zu %s", input_str, strlen(search), replace);

	// do the replacement
	replace_end(input_str, result, search, replace);

	printf("Replaced string: %s", result);
}