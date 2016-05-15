# gitdex.h

`gitdex.h` is a small, header only C library for parsing git index files (stored at *.git/index* in any git repository).

The binary format itself is specified in [git's technical documentation](https://github.com/git/git/blob/master/Documentation/technical/index-format.txt)

`demo.c` is an example program which will simply read and print out the index file.

# usage

In order to use gitdex, you must first have your file which you would like parsed
stored in a buffer. Gitdex also require that you know the size of the file.

```
// Parses the buffer into a gitdex_t structure.
gitdex_t *index;
index = gitdex_alloc(buffer, size);

// print the first entry
gitdex_print_entry(&index->entries[0])

// free the gitdex_t structure
gitdex_free(index);

```

# documentation

## functions

`gitdex_t *gitdex_alloc(void *src, size_t size);`

Will take a file buffer stored at `src`, of length `size` (in bytes) and return a pointer to parsed gitdex structure.


`void gitdex_free(gitdex_t *index);`

Free's the memory allocated by `gitdex_alloc`.


`size_t gitdex_read_index(gitdex_t *dest, void *src, size_t size);`

Parses a file buffer `src` of length `size` into a structure at `dest`.

Returns the size of the index (the size of the header plus the total size of all the entries as they were store in the original buffer).

`size_t gitdex_read_header(gitdex_header_t *dest, void *src);`

Parses a file buffer `src` into a structure at `dest`.

Returns the length of the header (alwayas 12 bytes).


`size_t gitdex_read_entry(gitdex_entry_t *dest, void *src);`

Parses an entry at `src` into a structure at `dest`.

Returns the length of the entry.


`int gitdex_check(void *src);`

Checks to see if a file buffer at `src` has a valid file signature.

Returns 1 or 0 if the file is valid or not respectively.


```
void gitdex_print_index(gitdex_t *index)
void gitdex_print_entry(gitdex_entry_t *entry)
void gitdex_print_header(gitdex_header_t *header)
```

Will pretty print parsed structures.

## types

**gitdex_header_t** stores the first 12 bytes of the index file, which always contains the signature, version and number of entries.

```
typedef struct {
	char signature[4];
	uint32_t version;
	uint32_t entries;
} gitdex_header_t;
```

**gitdex_entry_t** stores a single index entry, which contains everything git knows about a single file.

```
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
} gitdex_entry_t;
```

**gitdex_t** represents a whole index file consolidated into one structure, it contains the header information as well as a pointer to some arbitrary number of entries.

```
typedef struct {
	gitdex_header_t header;
	gitdex_entry_t  *entries;
} gitdex_t;

```

