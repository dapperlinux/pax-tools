/* rettofunc1.c - Tests whether return to function exploits work or not.
 *
 * Copyright (c)2003,2004 by Peter Busser <peter@adamantix.org>
 * This file has been released under the GNU Public Licence version 2 or later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "body.h"
#include "paxtest.h"

const char testname[] = "Return to function (strcpy, PIE)         ";

static const fptr overflow[32] = {
	itworked, itworked, itworked, itworked, itworked, itworked, itworked, itworked,
	itworked, itworked, itworked, itworked, itworked, itworked, itworked, itworked,
	itworked, itworked, itworked, itworked, itworked, itworked, itworked, itworked,
	itworked, itworked, itworked, itworked, itworked, itworked, itworked, NULL
};

void doit( void )
{
	char buf[sizeof(unsigned long)];

	if (strlen((const char *)overflow) > sizeof(overflow[0])) {
		forced_strcpy( buf, (const char *)overflow );
	} else {
		fprintf( stderr, "paxtest: return address contains a NULL byte.\n" );
		exit(1);
	}
}
