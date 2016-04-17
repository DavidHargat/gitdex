#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "gitdex-util.h"

// Constants for masking (&) 'mode' fields.
#define MODE_TYPE 0xF0000000
#define MODE_UNIX 0x1FF00000
#define MODE_TYPE_SHIFT 24
#define MODE_UNIX_SHIFT 16

// Constants for masking 'flag' fields.
#define FLAG_VALID    0x8000
#define FLAG_EXTENDED 0x4000
#define FLAG_STAGE    0x3000
#define FLAG_NAME     0x0FFF

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
	uint8_t mode[4];
	
	uint8_t uid[4];
	uint8_t gid[4];
	uint8_t size[4];
	uint8_t sha1[20];
	uint8_t flags[2];
	
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

void print_entry(entry_t *entry){	
	//char temp[64];
	size_t i;

	printf("%s\n",entry->name);
	printf("  ctime second: %d:%d\n", read_uint32(entry->ctime_second), read_uint32(entry->ctime_nanosecond));
	printf("  mtime second: %d:%d\n", read_uint32(entry->mtime_second), read_uint32(entry->mtime_nanosecond));
	
	printf("  dev: %d\t ino: %d\n", read_uint32(entry->dev), read_uint32(entry->ino));	
	printf("  uid: %d\t gid: %d\n", read_uint32(entry->uid), read_uint32(entry->gid));

	printf("  size: %d\n", read_uint32(entry->size));
	
	printf("%s","  SHA-1: ");
	for(i=0; i<20; i++)
		printf("%02x", entry->sha1[i]);
	printf("%s","\n");
}

void gitdex_parse(uint8_t *buf, size_t len){
	header_t *header;
	entry_t  *entry;

	header = (header_t *)(buf);	
	entry = (entry_t *)(((uint8_t *)header) + sizeof(header_t));
	
	printf("\nsignature: %.*s\n", 4, header->DIRC);
	printf("version:       %d\n", read_uint32(header->version));
	printf("entries:       %d\n\n", read_uint32(header->entries));
	
	print_entry(entry);
}

int main(int argc, char *argv[]){
	
	size_t len;
	uint8_t buffer[1024 * 2];
	
	if( !(len=file_buffer("./alpha/.git/index", buffer, 1024 * 2)) )
		die("failed to open file");

	gitdex_parse(buffer, len);
	
	return 0;
}
