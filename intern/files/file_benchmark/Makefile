COMPILER_FLAGS = "-Wall"
COMPILER_FLAGS += "-Wpedantic"
COMPILER_FLAGS += "-Wextra"

all: array_access.out file_access.out

array_access.out: array_access_benchmark.c find_op_array.c
	gcc ${COMPILER_FLAGS} $^ -o $@

FILE_LINES_LIBRARY = ../file_lines_offset/file_lines_offset.c

file_access.out: file_access_benchmark.c ${FILE_LINES_LIBRARY}
	gcc ${COMPILER_FLAGS} $^ -o $@
