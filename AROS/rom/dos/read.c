/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.7  1997/01/27 00:36:28  ldp
    Polish

    Revision 1.6  1996/12/09 13:53:38  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.5  1996/10/24 15:50:35  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.4  1996/09/13 17:50:08  digulla
    Use IPTR

    Revision 1.3  1996/08/13 13:52:50  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced AROS_LA by AROS_LHA

    Revision 1.2  1996/08/01 17:40:56  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH3(LONG, Read,

/*  SYNOPSIS */
	AROS_LHA(BPTR, file,   D1),
	AROS_LHA(APTR, buffer, D2),
	AROS_LHA(LONG, length, D3),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 7, Dos)

/*  FUNCTION
	Read some data from a given file. The request is directly
	given to the filesystem - no buffering is involved. For
	small amounts of data it's probably better to use the
	buffered I/O routines.

    INPUTS
	file   - filehandle
	buffer - pointer to buffer for the data
	length - number of bytes to read. The filesystem is
		 advised to try to fulfill the request as good
		 as possible.

    RESULT
	The number of bytes actually read, 0 if the end of the
	file was reached, -1 if an error happened. IoErr() will
	give additional information in that case.

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
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    /* Get pointer to filehandle */
    struct FileHandle *fh=(struct FileHandle *)BADDR(file);

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);

    /* Get pointer to I/O request. Use stackspace for now. */
    struct IOFileSys io,*iofs=&io;

    /* Prepare I/O request. */
    iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;
    iofs->IOFS.io_Message.mn_ReplyPort	 =&me->pr_MsgPort;
    iofs->IOFS.io_Message.mn_Length	 =sizeof(struct IOFileSys);
    iofs->IOFS.io_Device =fh->fh_Device;
    iofs->IOFS.io_Unit	 =fh->fh_Unit;
    iofs->IOFS.io_Command=FSA_READ;
    iofs->IOFS.io_Flags  =0;
    iofs->io_Args[0]=(IPTR)buffer;
    iofs->io_Args[1]=length;

    /* Send the request. */
    DoIO(&iofs->IOFS);

    /* Set error code and return */
    if((me->pr_Result2=iofs->io_DosError))
	return -1;
    else
	return iofs->io_Args[1];

    AROS_LIBFUNC_EXIT
} /* Read */
