#include <stdlib.h>
#include "mingw-lib.h"

void call_aligned_alloc()
{
	size_t size = 10, alignment = 16;
	void *ptr = _mm_malloc(size, alignment);

	_mm_free(ptr);
}