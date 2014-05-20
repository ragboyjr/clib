#ifndef _LIB_HASH_H
#define _LIB_HASH_H

/* simple byte hashes implementing the djb2 algorithm */

/*
 * TODO - Implement Murmur3 hashing algorithm along with MSCV (MicroSoftVisualStudio) compatibility for inttypes.
 * see https://github.com/jemalloc/jemalloc for decent implementation of the murmur hash along with MSVC compatibility.
 */

/*
 * Accepts a null-terminated string to hash
 */
unsigned long hash_str(char * str);

/*
 * Accepts a pointer to literally anything, and it hashes
 * len bytes.
 */
unsigned long hash_bytes(void * vdata, int len);

#endif
