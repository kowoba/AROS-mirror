/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.1  1996/08/15 13:24:20  digulla
    New function: kprintf() allows to print a text which is always shown to the
    user no matter what.

    Revision 1.1  1996/08/01 18:46:31  digulla
    Simple string compare function

    Desc:
    Lang:
*/
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <aros/system.h>
#include <clib/dos_protos.h>

extern struct DosBase * DOSBase;

/*****************************************************************************

    NAME */
	#include <clib/aros_protos.h>

	int kprintf (

/*  SYNOPSIS */
	const UBYTE * fmt,
	...)

/*  FUNCTION
	Formats fmt with the specified arguments like printf() (and *not*
	like RawDoFmt()) and uses a secure way to deliver the message to
	the user; ie. the user *will* see this message no matter what.

    INPUTS
	fmt - printf()-style format string

    RESULT
	The number of characters output.

    NOTES
	This function is not part of a library and may thus be called
	any time.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	24-12-95    digulla created

******************************************************************************/
{
    va_list args;
    ULONG   vpargs[10];
    int     t;
    const char * ptr;
    int     ret;

    va_start (args, fmt);

    for (t=0,ptr=fmt; t<10; )
    {
	while (*ptr)
	{
	    if (*ptr == '%')
	    {
		while (isdigit(*ptr) || *ptr=='.' || *ptr=='-')
		    ptr ++;

		switch (*ptr)
		{
		case '%': break;
		case 'l':
		    if (ptr[1] == 'd' || tolower(ptr[1]) == 'x')
			ptr ++;

		    vpargs[t ++] = va_arg (args, ULONG);
		    break;
		default:
		    vpargs[t ++] = va_arg (args, int); break;
		    break;
		}
	    }

	    ptr ++;
	}
    }
    ret = VPrintf ((char *)fmt, vpargs);
    Flush (Output ());

    va_end (args);

    return ret;
} /* kprintf */

