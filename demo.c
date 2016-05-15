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
 * defined as gitdex_t where you will have access to the header and entries.
 * 
 * The simplest way to test is to just print them, as is done in this example.
 */

#define MAX_SIZE (4096)
#define MAX_ENTRIES (32)

// Example in heap memory (malloc()'d and free()'d).
void example_heap(void *data, size_t len){
	gitdex_t *index = gitdex_alloc(data, len);
	gitdex_print_index(index);
	gitdex_free(index);	
}

// Example entirely in stack memory.
//   Since the number of entries is non-deterministic,
//   we decide how many entries we can store (MAX_ENTRIES)
//   and assign an appropriate buffer to the index.entries pointer.
void example_stack(void *data, size_t len){
	gitdex_t  index;
	char buffer[sizeof(gitdex_entry_t) * MAX_ENTRIES];
	
	index.entries = buffer;
	
	gitdex_read_index(&index, data, len);
	gitdex_print_index(&index);
}

void print_index(char *filename){
	size_t len;
	unsigned char data[MAX_SIZE];
	
	// Reads a file to a buffer
	if( !(len=file_buffer(filename, data, MAX_SIZE)) )
		die("error: file not found");

	if( gitdex_check(data) ){
		example_stack(data, len);
		//example_heap(data, len);
	}else{
		die("error: not a git index file");
	}
}

int main(int argc, char **argv){

	if(argc == 2)
		print_index(argv[1]);
	else
		printf("%s", 
			"usage: demo <filename>\n\n"
			"try doing `./demo .git/index` in a git directory.\n"
		);

	return 0;
}
