/* mprotstack.c - Tests whether code on the stack can be executed after trying
 *                to make it executable by using mprotect().
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

const char testname[] = "Executable stack (mprotect)              ";

void doit( void )
{
	char buf[MAX_SHELLCODE_LEN];
	fptr func;

	copy_shellcode(buf);

	/* Convert the pointer to a function pointer */
	func = (fptr)&buf;

	/* Try to make the stack executable first */
	do_mprotect( &buf, sizeof(buf), PROT_READ|PROT_WRITE|PROT_EXEC );

	/* Call the code in the buffer */
	func();

	/* It worked when the function returns */
	itworked();
}
