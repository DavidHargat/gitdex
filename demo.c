#include <stdlib.h>
#include <stdio.h>

#include "gitdex.h"
#include "util.h"

/* This is a demo of gitdex.h which shows how the 
 * interface works, and how you can work on either
 * the stack or heap.
 *
 * (util.h is simply a utility for reading files, it
 * does NOT have to be included with gitdex.h)
 *
 * With gitdex you can use either gitdex_alloc(src, size) or 
 * gitdex_read_index(src, size).
 *   `src`  is a buffer of raw data, containing your index file.
 *   `size` is a file length.
 *
 * In either case gitdex will parse your buffered file into a struct
 * defined as index_t where you will have access to the header and entries.
 * 
 * The simplest way to test is to just print them, as is done in this example.
 */

#define MAX_SIZE (4096)

// Example in heap memory (malloc()'d and free()'d).
void example_heap(void *data, size_t len){
	index_t *index = gitdex_alloc(data, len);
	gitdex_print_index(index);
	gitdex_free(index);	
}

// Example entirely in stack memory.
void example_stack(void *data, size_t len){
	index_t  index;
	header_t header;
	entry_t  entries[32];

	index.header  = &header;
	index.entries = entries;
	
	gitdex_read_index(&index, data, len);
	gitdex_print_index(&index);
}

int main(int argc, char **argv){
	size_t len;
	unsigned char data[MAX_SIZE];
	char *filename;
	
	if(argc > 1)
		filename = argv[1];
	else
		filename = ".git/index";

	// Reads a file to a buffer
	if( !(len=file_buffer(filename, data, MAX_SIZE)) )
		die("failed to open file");

	// Each example reads from the buffer into an `index_t` and prints it.
	//example_stack(data, len);
	example_heap(data, len);

	return 0;
}
