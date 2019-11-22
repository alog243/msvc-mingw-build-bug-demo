### MinGW-built library / MSVC linker bug demo

I ran into this bug by accident and it boggles my mind why MSVC behaves this way. I built a bare-bones project demonstrating the bug's behavior.

Using:
- Visual Studio Enterprise 2019 Version 16.3.7
- MinGW gcc version 9.2.0

#### Summary

MinGW builds static libraries `libname.a` that are more-or-less compatible with MSVC and can be linked against in the MSVC toolchain. If this library is built to statically link against the standard library with `-static` then in some cases it will still import references to libc functions as `__imp__[function name]` in the object file.

This isn't a huge problem as you can usually define the missing references yourself and call the proper function as a workaround. In the example of this project, the MinGW-built library calls `_mm_alloc()` and `_mm_free()` which get built with references to `__imp__aligned_malloc()` and `__imp__aligned_free()`. These functions are not available in msvc and are gcc-specific. The MSVC equivalent of `aligned_malloc()` is `_aligned_malloc()`, which takes the same parameters but in different order.

The bug happens when you try to build a function that points to MSVC's `_aligned_malloc()`. There is some kind of behavior in MSVC to detect `__imp__[name]` symbols as imported functions, but it seems this behavior isn't well defined. This leads to situations where functions are recognized, improperly linked, and forced to crash due to MSVC's linker.

The following programs in the project demonstrate each scenario I found while testing this behavior.

#### Disclaimer

Yes, I agree, linking a MinGW-built static library into the MSVC buildchain is absurd and should never be done. I personally ran into this because MSVC doesn't implement all of the C99 standard for complex numbers so I had a project that could only be built in GCC, but I prefer developing in Visual Studio so I made a disgusting workaround to stay in my preferred build and debug enviroment. Moving on ...

#### Program 1: `msvc-main-nobuild` - This does not build

`mingw-lib`:
- `void call_aligned_alloc()` - this just calls `_mm_malloc()` and `_mm_free()` and exits. MinGW builds this to reference `__imp__aligned_malloc` and `__imp__aligned_free`.

`msvc-lib`:
- `void call_mingw_lib()` - simply calls the above function

`msvc-main`: calls `call_mingw_lib()`

	LNK2019: unresolved external symbol __imp__aligned_malloc referenced in function call_aligned_alloc
	LNK2019 unresolved external symbol __imp__aligned_free referenced in function call_aligned_alloc
	LNK1120 2 unresolved externals

#### Program 2: `msvc-main-bug[1-3]` - This builds but will *always* crash

`mingw-lib`: same as previous

`msvc-main`: same as previous

`msvc-lib`:
- `msvc-lib.c: void call_mingw_lib()`
- `msvc-bug[1-3].c` - one of the following 3 files:

```c
// msvc-bug1.c
#include <stdlib.h>

void *__imp__aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void __imp__aligned_free(void *block)
{
	_aligned_free(block);
}
```

```c
// msvc-bug2.c
#include <stdlib.h>

void *aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void __imp__aligned_free(void *block)
{
	_aligned_free(block);
}
```

```c
// msvc-bug3.c
#include <stdlib.h>

void *__imp__aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void aligned_free(void *block)
{
	_aligned_free(block);
}
```

*ANY* of these files resolves the unresolved external symbol references ... whether the actual `__imp__` prefix is present or not. It seems the linker has some logic for detecting that `aligned_malloc` is supposed to resolve the reference to `__imp__aligned_malloc`. The odd/ridiculous thing is that it won't recognize it if there are *no* `__imp__` prefixes in the object file. This is demonstrated in the next program.

The reason this is a bug is because the functions with the `__imp__` prefix get improperly linked in MSVC and will always crash when called from `mingw-lib`.

Here's what's built from MinGW:

```
call_aligned_alloc:
	sub rsp, 28h
	mov edx, 10h
	mov ecx, 0Ah
	call qword ptr [__imp__aligned_malloc (07FF7EB401F6Eh)]
	mov rcx, rax
	add rsp, 28h
	jmp qword ptr [__imp__aligned_free (07FF7EB402D9Ch)]
```

And here is what MSVC sticks in `07FF7EB401F6Eh`:
```
00007FF7EB401F6E	jmp __imp__aligned_malloc (07FF7EB466300h)
```
The observant reader will recognize the issue here. Honestly this is as much MinGW's fault as it is MSVC's. Instead of an address being written here it's a `jmp` instruction, so rather than calling `__imp__aligned_malloc (07FF7EB466300h)` it's calling some assembly code and trying to treat it as a function pointer. This ... crashes, unequivocally.

Now take a look at what happens if I define `aligned_malloc()` instead of `__imp__aligned_malloc()`.

```
call_aligned_alloc:
	...
	call qword_ptr [__imp__aligned_malloc (07FF64E48ADB0h)]
	...
```

```
00007FF64E48ADB0	EF 20 41 4E F6 7F 00
```
This is a memory address! Now instead of calling assembly code as a function pointer, it's calling:
```
00007FF64E4120EF	jmp _aligned_malloc (07FF64E477F50h)
```

Which is exactly what we wanted. This now runs properly and behaves exactly as it should behave. However, since we still have `__imp__aligned_free()`, this behaves as the previous example and crashes our process.

So if having `__imp__` causes the function to not work, why not remove them? This brings me to:

#### Program 3: `msvc-main-bug-nobuild`: This does not build

`mingw-lib`: same as before

`msvc-main`: same as before

`msvc-lib`:
- `msvc-lib.c: void call_mingw_lib()`
- `msvc-bug4-nobuild.c` - Contents below

```c
// msvc-bug4-nobuild.c
#include <stdlib.h>

void *aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void aligned_free(void *block)
{
	_aligned_free(block);
}
```

This makes little sense. The linker now doesn't recognize these functions as the exported versions of `__imp__aligned_malloc` and `__imp__aligned_free` anymore. This results in the same unresolved external errors as the first program.

So is there a way to get around this? I'm sorry you asked.

#### Program 5: `msvc-main-hack` - ?????

Our goal here is to somehow get MSVC to recognize that our functions are imported functions without using the improper linkage that crashes the process. By some fluke I found that if you export a *third* function with an `__imp__` prefix in the same object file, then call that function from some other object file in the lib, AND make sure any function from that other object file is called in your code (so that the object file is pulled in by the linker), then everything "works".

`mingw-lib`: unchanged

`msvc-lib`: this lib now looks a little wacky
- `msvc-lib.c`: unchanged
- `msvc-bug5-hack.c`
- `msvc-bug5-extrafile.c`
- `msvc-bug5-extrafile.h`

`msvc-main`: instead of calling `call_mingw_lib()` it calls `exposed_function()` first (to make sure the linker pulls in `msvc-bug5-extrafile.obj`)

```c
// msvc-bug5-hack.c
#include <stdlib.h>

void *aligned_malloc(size_t alignment, size_t size)
{
	return _aligned_malloc(size, alignment);
}

void aligned_free(void *block)
{
	_aligned_free(block);
}

void *__imp__aligned_realloc(void *block, size_t alignment, size_t size)
{
	return _aligned_realloc(block, size, alignment);
}
```

```c
// msvc-bug5-extrafile.c
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
```

```c
// msvc-bug5-extrafile.h
#ifndef MSVC_BUG_5_EXTRAFILE_H
#define MSVC_BUG_5_EXTRAFILE_H

int exposed_function(int x);

#endif
```

Things to note:
1. `dummy_function()` is literally never called, there is literally no way to call it from anywhere. The only requisite is that a reference to the extra `__imp__` function is present in that object file
2. A function from that object file *has* to be called in the final executable. In this case I used `exposed_function()`
3. This builds and runs and does not crash, and both `aligned_malloc()` and `aligned_free()` are properly redirected to their MSVC equivalents

Why does this happen? No clue.

I built this using a CMake project in Visual Studio; all the targets are already set up to build and run. The library I built with MinGW is included in this repo, or if you would like to rebuild it yourself the Makefile is included.