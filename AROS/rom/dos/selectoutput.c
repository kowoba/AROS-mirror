/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.3  1996/08/13 13:52:51  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced __AROS_LA by __AROS_LHA

    Revision 1.2  1996/08/01 17:40:58  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <clib/exec_protos.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
	#include <clib/dos_protos.h>

	__AROS_LH1(BPTR, SelectOutput,

/*  SYNOPSIS */
	__AROS_LHA(BPTR, fh, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 50, Dos)

/*  FUNCTION
	Sets the current output stream returned by Output() to a new
	value. Returns the old output stream.

    INPUTS
	fh - New output stream.

    RESULT
	Old output stream handle.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct DosLibrary *,DOSBase)

    BPTR old;

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);

    /* Nothing spectacular */
    old=me->pr_COS;
    me->pr_COS=fh;
    return old;
    __AROS_FUNC_EXIT
} /* SelectOutput */
