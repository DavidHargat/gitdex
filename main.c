#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "util.h"

// Constants for masking 'flag' fields.
#define FLAG_VALID     (1 << 16)
#define FLAG_EXTENDED  (1 << 15)
#define FLAG_STAGE    ((1 << 14) | (1 << 13))
#define FLAG_NAME      (0xFFF)
#define FLAG_ALL     (FLAG_VALID | FLAG_EXTENDED | FLAG_STAGE)

// Number of extension signatures.
#define EXT_NUM  3

// Extension signatures are 4 bytes long.
#define EXT_SIZE 4

typedef enum {
	EXT_TREE,
	EXT_LINK,
	EXT_UNTR
} ext_t;

// Extension signatures.
static char *extensions[EXT_NUM] = {
	(char []) {'T', 'R', 'E', 'E'},
	(char []) {'l', 'i', 'n', 'k'},
	(char []) {'U', 'N', 'T', 'R'}
};

// File Header
typedef struct {
	uint8_t  DIRC[4];
	uint32_t version;
	uint32_t entries;
} header_t;

// Version 2 as specified in index-format.txt
typedef struct {
	uint32_t ctime_second;
	uint32_t ctime_nanosecond;
	uint32_t mtime_second;
	uint32_t mtime_nanosecond;
	uint32_t dev;
	uint32_t ino;
	uint32_t mode;
	uint32_t uid;
	uint32_t gid;
	uint32_t size;
	uint8_t sha1[20];
	uint16_t flags;
	char name[1];
} entry_t;

/* All integer values must be converted to host order
* else our bitwise ops are meaningless.
* They are naturally network order (index-format:6).
*/
void header_ntoh(header_t *header){
	header->version = ntohl(header->version);
	header->entries = ntohl(header->entries);
}

void entry_ntoh(entry_t *entry){
	entry->ctime_second     = ntohl(entry->ctime_second);
	entry->ctime_nanosecond = ntohl(entry->ctime_nanosecond);
	entry->mtime_second     = ntohl(entry->mtime_second);
	entry->mtime_nanosecond = ntohl(entry->mtime_nanosecond);
	entry->dev              = ntohl(entry->dev);
	entry->mode             = ntohl(entry->mode);
	entry->ino              = ntohl(entry->ino);
	entry->uid              = ntohl(entry->uid);
	entry->gid              = ntohl(entry->gid);
	entry->size             = ntohl(entry->size);
	entry->flags            = ntohs(entry->flags);
}

// returns 1 on match, else 0
char chars_match(char *a, char *b, size_t len){
	size_t i;
	for(i=0; i<len; i++)
		if(a[i] != b[i])
		return 0;	
	return 1;
}

/* Maps an extensions signature to the `ext_t` enum.
 * returns 
 *   success: index of signature
 *   error:   -1
 */
ext_t read_ext(char *src){
	size_t i;
	for(i=0; i<EXT_NUM; i++)
		if(chars_match(extensions[i], src, EXT_SIZE))
		return i;
	return -1;
}

void print_sha1(unsigned char *src){
	size_t i;
	printf("%s","SHA-1: ");
	for(i=0; i<20; i++)
		printf("%02x", src[i]);
	printf("%s","\n");
}

void print_entry(entry_t *entry){	
	size_t i;

	printf("%06o %s\n", entry->mode, entry->name);
	printf("  ctime: %d:%d\n", entry->ctime_second, entry->ctime_nanosecond);
	printf("  mtime: %d:%d\n", entry->mtime_second, entry->mtime_nanosecond);
	
	printf("  dev: %d\tino: %d\n", entry->dev, entry->ino);	
	printf("  uid: %d\tgid: %d\n", entry->uid, entry->gid);

	printf("  size: %d\tflags: %d\n", entry->size, entry->flags & FLAG_ALL);

	printf("  ");
	print_sha1(entry->sha1);

	printf("%s","\n");
}

void print_header(header_t *header){
	size_t entries, version;
	
	entries = header->entries;
	version = header->version;

	printf("\nsignature:\t%.4s\n", header->DIRC);
	printf("version:\t%lu\n",      version);
	printf("entries:\t%lu\n\n",    entries);
}

void gitdex_parse(uint8_t *buf, size_t len){
	header_t *header;
	entry_t  *entry;
	size_t offset, i;

	header = (header_t *)(buf);	
	header_ntoh(header);
	print_header(header);

	// TODO: implement version 3
	//if( read_uint32(header->version) != 2 ) die("Unsupported version.");

	offset = sizeof(header_t);

	for(i = 0; i < header->entries; i++){
		size_t entry_length;
		
		entry = buf + offset;
		entry_ntoh(entry);
	
		// We have to subtract 1 here since the null byte is a part of the padding.
		// If the entry isn't divisible by 8, pad the offset (index-format.txt:126)
		
		entry_length = sizeof(entry_t) + strlen(entry->name) - 1;
			
		if( entry_length % 8 )
			offset += 8 - (entry_length % 8);

		offset += entry_length;
		
		print_entry(entry);
	}

	// Only 20 bytes left? We're at the final sha1. (index-format.txt:35)
	if( (offset+20) == len ){
		print_sha1(buf + offset);
		return;
	}
	
	ext_t ext;
	ext = read_ext((char *)(buf + offset));
	
	switch(ext){
		case EXT_TREE:
		case EXT_LINK:
		case EXT_UNTR:
			printf("extension: %.4s\n", extensions[ext]);
			break;
		default:
			printf("Unknown extension '%.4s'\n", (char *)(buf+offset));
	}
}

int main(int argc, char *argv[]){
	size_t len;
	uint8_t buffer[1024 * 2];
	
	if( !(len=file_buffer("test.index", buffer, 1024 * 2)) )
		die("failed to open file");

	gitdex_parse(buffer, len);
	
	return 0;
}
