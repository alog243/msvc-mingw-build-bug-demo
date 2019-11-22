// msvc-main.c - demonstrates linker bug with MinGW-compiled libraries
//

#include <stdlib.h>

void *__imp__aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void __imp__aligned_free(void *block)
{
	_aligned_free(block);
}