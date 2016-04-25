#ifndef GITDEX_H
#define GITDEX_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define NAME_MAX (255)
#define SHA1_SIZE (20)

#define FLAG_VALID     (1 << 15)
#define FLAG_EXTENDED  (1 << 14)
#define FLAG_STAGE    ((1 << 13) | (1 << 12))
#define FLAG_NAME      (0xFFF)
#define FLAG_ALL     (FLAG_VALID | FLAG_EXTENDED | FLAG_STAGE)

typedef struct {
	char signature[4];
	uint32_t version;
	uint32_t entries;
} header_t;

typedef struct {
	uint32_t ctime_second;
	uint32_t ctime_nano;
	uint32_t mtime_second;
	uint32_t mtime_nano;
	uint32_t dev;
	uint32_t ino;
	uint32_t mode;
	uint32_t uid;
	uint32_t gid;
	uint32_t size;
	uint8_t  sha1[SHA1_SIZE];
	uint16_t flags;
	uint16_t extended;
	char name[NAME_MAX];
} entry_t;

typedef struct {
	header_t *header;
	entry_t  *entries;
} index_t;

index_t *gitdex_alloc(void *src, size_t size);
void     gitdex_free(index_t *index);
size_t   gitdex_read_index(index_t *dest, void *src, size_t size);
size_t   gitdex_read_header(header_t *dest, void *src);
size_t   gitdex_read_entry(entry_t *dest, void *src);

void gitdex_print_index(index_t *index);
void gitdex_print_entry(entry_t *entry);
void gitdex_print_header(header_t *header);

index_t *gitdex_alloc(void *src, size_t size){
	index_t *index;

	index         = malloc(sizeof(index_t));
	index->header = malloc(sizeof(header_t));
	
	gitdex_read_header(index->header, src);
	
	index->entries = malloc(sizeof(entry_t) * index->header->entries);

	gitdex_read_index(index, src, size);

	return index;
}

void gitdex_free(index_t *index){
	free(index->header);
	free(index->entries);
	free(index);
}

uint32_t gitdex_read_uint32(uint32_t *src){
	return ntohl(*src);
}

uint16_t gitdex_read_uint16(uint16_t *src){
	return ntohs(*src);
}

size_t gitdex_read_index(index_t *dest, void *src, size_t size){
	size_t i, offset;
	offset=0;
	
	offset += gitdex_read_header(dest->header, src);
	for(i=0; i < (dest->header->entries); i++)
		offset += gitdex_read_entry( &dest->entries[i], src+offset);
	
	return offset;
}

size_t gitdex_read_header(header_t *dest, void *src){
	size_t offset;
	offset = 0;
	memcpy(dest->signature, src + offset, 4);
	offset += 4;
	dest->version = gitdex_read_uint32(src + offset);
	offset += 4;
	dest->entries = gitdex_read_uint32(src + offset); 
	offset += 4;
	return offset;
}

// Returns the size of the entry.
size_t gitdex_read_entry(entry_t *dest, void *src){
	size_t offset;
	size_t namelen;
	offset = 0;

	dest->ctime_second = gitdex_read_uint32(src + offset); offset += 4;
	dest->ctime_nano   = gitdex_read_uint32(src + offset); offset += 4;
	dest->mtime_second = gitdex_read_uint32(src + offset); offset += 4;
	dest->mtime_nano   = gitdex_read_uint32(src + offset); offset += 4;
	dest->dev          = gitdex_read_uint32(src + offset); offset += 4;
	dest->ino          = gitdex_read_uint32(src + offset); offset += 4;
	dest->mode         = gitdex_read_uint32(src + offset); offset += 4;
	dest->uid          = gitdex_read_uint32(src + offset); offset += 4;
	dest->gid          = gitdex_read_uint32(src + offset); offset += 4;
	dest->size         = gitdex_read_uint32(src + offset); offset += 4;

	memcpy(dest->sha1, src + offset, SHA1_SIZE);
	offset += SHA1_SIZE;

	dest->flags = gitdex_read_uint16(src + offset);
	offset += 2;

	if(dest->flags & FLAG_EXTENDED){
		dest->extended = gitdex_read_uint16(src + offset);
		offset += 2;
	}

	namelen = strlen(src + offset);

	// If the name exceeds NAME_MAX, only read the first 255 bytes
	if(namelen < NAME_MAX)
		strcpy(dest->name, src + offset);
	else
		strncpy(dest->name, src + offset, NAME_MAX);

	offset += namelen + 1; // +1 for the nullbyte in the padding.
	
	if( offset % 8 )
		offset += 8 - (offset % 8);

	return offset;
}

void gitdex_print_index(index_t *index){
	size_t i;
	gitdex_print_header(index->header);
	for(i=0; i < (index->header->entries); i++)
		gitdex_print_entry(&index->entries[i]);
}

void gitdex_print_header(header_t *header){
	printf("signature: %.4s\n", header->signature);
	printf("version:   %u\n",  header->version);
	printf("entries:   %u\n",  header->entries);
}

void gitdex_print_entry(entry_t *entry){
	size_t i;
	printf("%s\n", entry->name);
	printf("  mode: %06o\n", entry->mode);
	printf("  ctime: %d:%d\n", entry->ctime_second, entry->ctime_nano);
	printf("  mtime: %d:%d\n", entry->mtime_second, entry->mtime_nano);
	printf("  dev: %d\tino: %d\n", entry->dev, entry->ino);	
	printf("  uid: %d\tgid: %d\n", entry->uid, entry->gid);
	printf("  size: %d\tflags: %d\n", entry->size, entry->flags & FLAG_ALL);
	printf("%s","  SHA-1: ");
	for(i=0; i<20; i++) printf("%02x", entry->sha1[i]);
	printf("%s","\n");
}

#endif
