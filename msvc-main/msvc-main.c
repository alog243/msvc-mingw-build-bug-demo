// msvc-main.c - demonstrates linker bug with MinGW-compiled libraries
//

#include "msvc-lib.h"

int main()
{
	// This will trigger a crash because the "hack-fix" file causes the functions to be improperly linked in the binary
	call_mingw_lib();
	return 0;
}
