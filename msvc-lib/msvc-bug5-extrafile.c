#define NULL 0

#include "msvc-bug5-extrafile.h"

void dummy_function()
{
	void *fake_ptr = __imp__aligned_realloc(NULL, 8, 8);
}

int exposed_function(int x)
{
	return 2 * x;
}