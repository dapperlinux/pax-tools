/* getshlibdelta.c - Get the delta between a function in .text and a function in a
 *                   shared library and print it
 *
 * Copyright (c)2003 by Peter Busser <peter@adamantix.org>
 * Copyright (c)2014 by Kees Cook <keescook@chromium.org>
 * This file has been released under the GNU Public Licence version 2 or later
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

/* OpenBSD 3.5 doesn't define RTLD_DEFAULT */
/* OpenBSD 3.6 does but it doesn't actually handle (segfaults on) RTLD_DEFAULT, sigh... */
#ifdef __OpenBSD__
#undef RTLD_DEFAULT
#define RTLD_DEFAULT "libc.so"
#endif

void __attribute__ ((noinline)) foo(void)
{
	unsigned long ptr = (unsigned long)__builtin_return_address(0);
	void *handle;

	handle = dlopen( RTLD_DEFAULT, RTLD_LAZY );
	if (handle != NULL) {
		unsigned long sprintf;

		dlerror(); /* clear any errors */
		sprintf = (unsigned long)dlsym( handle, "sprintf" );
		if (dlerror() == NULL) {
			printf( "%p\n", (void *)(ptr - sprintf) );

		}

		dlclose( handle );
	}
}

int main(int argc, char *argv[])
{
	foo();
	exit(0);
}
