#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "util.h"

// Constants for masking 'flag' fields.
#define FLAG_VALID     (1 << 16)
#define FLAG_EXTENDED  (1 << 15)
#define FLAG_STAGE    ((1 << 14) | (1 << 13))
#define FLAG_NAME      (0x0FFF)

// Excludes the flag name field
#define FLAG_ALL     (FLAG_VALID | FLAG_EXTENDED | FLAG_STAGE)

// Permissions are coerced into 0755 or 0644
#define PERM(mode) (((mode) & 0100) ? 0755 : 0644)

// Number of extension signatures.
#define EXT_NUM  3

// Extension signatures are 4 bytes long.
#define EXT_SIZE 4

typedef enum {
	EXT_UNKNOWN=-1,
	EXT_TREE=0,
	EXT_LINK=1,
	EXT_UNTR=2
} ext_t;

// Extension signatures.
static char *extensions[EXT_NUM] = {
	(char []) {'T', 'R', 'E', 'E'},
	(char []) {'l', 'i', 'n', 'k'},
	(char []) {'U', 'N', 'T', 'R'}
};

// File Header
typedef struct {
	uint8_t DIRC[4];
	uint8_t version[4];
	uint8_t entries[4];
} header_t;

// Version 2 as specified in index-format.txt
typedef struct {
	uint8_t ctime_second[4];
	uint8_t ctime_nanosecond[4];
	uint8_t mtime_second[4];
	uint8_t mtime_nanosecond[4];
	uint8_t dev[4];
	uint8_t ino[4];
	
	uint32_t mode;
	
	uint8_t uid[4];
	uint8_t gid[4];
	uint8_t size[4];
	
	uint8_t sha1[20];
	
	uint16_t flags;
	
	char name[1];
} entry_t;

uint32_t read_uint32(uint8_t *src){
	uint32_t value;
	value = 0;
	value += (src[3]);
	value += (src[2]) << 8;
	value += (src[1]) << 16;
	value += (src[0]) << 24;
	return value;
}

uint16_t read_uint16(uint8_t *src){
	uint16_t value;
	value = 0;
	value += (src[1]);
	value += (src[0]) << 8;
	return value;
}

char chars_match(char *a, char *b, size_t len){
	size_t i;

	for(i=0; i<len; i++)
		if(a[i] != b[i])
		return 0;
	
	return 1;
}

/* returns 
 *   success: index of signature
 *   error:   -1
 */
ext_t read_ext(char *src){
	size_t i;
	for(i=0; i<EXT_NUM; i++)
		if(chars_match(extensions[i], src, EXT_SIZE))
		return i;
	return EXT_UNKNOWN;
}

void epoch_to_string(char *buf, size_t size, time_t time){
	struct tm *timeinfo;
	timeinfo = localtime(&time);
	strftime(buf, size, "%a %Y-%m-%d %H:%M:%S %Z", timeinfo);
}

/*
epoch_to_string(temp, 64, read_uint32(entry->ctime_second));
printf("ctime second:  %d (%s)\n", read_uint32(entry->ctime_second), temp);
printf("ctime nano:    %d\n", read_uint32(entry->ctime_nanosecond));
epoch_to_string(temp, 64, read_uint32(entry->mtime_second));
printf("mtime second:  %d (%s)\n", read_uint32(entry->mtime_second), temp);
printf("mtime nano:    %d\n\n", read_uint32(entry->mtime_nanosecond));
*/

void print_sha1(unsigned char *src){
	size_t i;
	printf("%s","SHA-1: ");
	for(i=0; i<20; i++)
		printf("%02x", src[i]);
	printf("%s","\n");
}

void print_entry(entry_t *entry){	
	//char temp[64];
	size_t i;

	printf("%06o %s\n", PERM(entry->mode), entry->name);
	printf("  ctime: %d:%d\n", read_uint32(entry->ctime_second), read_uint32(entry->ctime_nanosecond));
	printf("  mtime: %d:%d\n", read_uint32(entry->mtime_second), read_uint32(entry->mtime_nanosecond));
	
	printf("  dev: %d\tino: %d\n", read_uint32(entry->dev), read_uint32(entry->ino));	
	printf("  uid: %d\tgid: %d\n", read_uint32(entry->uid), read_uint32(entry->gid));

	printf("  size: %d\tflags: %x\n", read_uint32(entry->size), (entry->flags & FLAG_ALL));

	printf("  ");
	print_sha1(entry->sha1);

	printf("%s","\n");
}

void print_header(header_t *header){
	size_t entries, version;
	
	entries = read_uint32(header->entries);
	version = read_uint32(header->version);

	printf("\nsignature:\t%.*s\n", 4, header->DIRC);
	printf("version:\t%lu\n",   version);
	printf("entries:\t%lu\n\n", entries);
}

void gitdex_parse(uint8_t *buf, size_t len){
	header_t *header;
	entry_t  *entry;
	
	size_t offset, i, entries;

	// [HEADER]
	header = (header_t *)(buf);	
	offset = sizeof(header_t);
	
	print_header(header);

	// TODO: implement version 3
	//if( read_uint32(header->version) != 2 ) die("Unsupported version.");

	entries = read_uint32(header->entries);

	// [ENTRIES]
	for(i = 0; i < entries; i++){
		size_t entry_length;
		
		entry = buf + offset;
		
		// We have to subtract 1 here since the null byte is a part of the padding.
		// If the entry isn't divisible by 8, pad the offset (index-format.txt:126)
		
		entry_length = sizeof(entry_t) + strlen(entry->name) - 1;
			
		if( entry_length % 8 )
			offset += 8 - (entry_length % 8);

		offset += entry_length;
		
		print_entry(entry);
	}

	// [EXTENSIONS]

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
	
	if( !(len=file_buffer("alpha/.git/index", buffer, 1024 * 2)) )
		die("failed to open file");

	gitdex_parse(buffer, len);
	
	return 0;
}
