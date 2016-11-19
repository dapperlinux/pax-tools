/* mprotdata.c - Tests whether code in the .data segment can be executed after
 *               trying to use mprotect() to make it executable.
 *
 * Copyright (c)2003 by Peter Busser <peter@adamantix.org>
 * This file has been released under the GNU Public Licence version 2 or later
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "body.h"
#include "shellcode.h"

const char testname[] = "Executable data (mprotect)               ";

char buf[PAGE_SIZE_MAX] __pagealigned = { 'A' };

void doit( void )
{
	fptr func;

	copy_shellcode(buf);

	/* Convert the pointer to a function pointer */
	func = (fptr)&buf;

	/* Try to make the data executable first by using mprotect */
	/* Due to an OpenBSD bug PROT_READ is required */
	do_mprotect( &buf, sizeof(buf), PROT_READ|PROT_EXEC );

	/* Call the code in the buffer */
	func();

	do_mprotect( &buf, sizeof(buf), PROT_READ|PROT_WRITE );

	/* It worked when the function returns */
	itworked();
}
