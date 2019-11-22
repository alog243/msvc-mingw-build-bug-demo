// msvc-main.c - demonstrates linker bug with MinGW-compiled libraries
//

#include "msvc-lib.h"
#include "msvc-bug5-extrafile.h"

int main()
{
	// This will work because the linker magically recognizes the __imp__ functions due to the hack
	int i = exposed_function(15);
	call_mingw_lib();
	return 0;
}
