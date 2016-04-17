#ifndef GITDEX_UTIL_H
#define GITDEX_UTIL_H

#include <stdlib.h>
#include <stdio.h>

/* Determines the total number of bytes from file `f`.
 * return:
 *     success: file length
 *     error:   0
 */
size_t file_length(FILE *f){
	long len;

	if( ( fseek(f, 0L, SEEK_END)<0 ) ||
		( (len=ftell(f))<0 ) ||
		( fseek(f, 0L, SEEK_SET)<0 )
	)
		return 0;
	
	return (size_t) len;
}

/* Reads `length` bytes from file `f` to destination `dest`.
 * return:
 *     success:  1
 *     error:    0
 */
char file_read(void *dest, size_t length, FILE *f){
	if(fread(dest, 1, length, f)==length)
		return 1;
	else
		return 0;
}

/* Opens a file and reads it to a buffer.
 * return:
 *     success: length of the file
 *     error:   0
 */
size_t file_buffer(char *filename, void *dest, size_t max){
	FILE *f;
	size_t len;
	
	if( !(f = fopen(filename, "r")) ||
		!(len = file_length(f)) ||
		(len > max) ||
		!(file_read(dest, len, f))
	)
		len = 0;
		
	fclose(f);

	return len;
}

void die(char *reason){
	printf("%s\n", reason);
	exit(EXIT_FAILURE);
}

#endif

