/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: AROSList initialization code.
    Lang: English.
*/

#include <stddef.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "aroslist_intern.h"

#undef SysBase

/* Customize libheader.c */
#define LC_SYSBASE_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->sysbase)
#define LC_SEGLIST_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->seglist)
#define LC_LIBBASESIZE          sizeof(LIBBASETYPE)
#define LC_LIBHEADERTYPEPTR     LIBBASETYPEPTR
#define LC_LIB_FIELD(lib)       (((LIBBASETYPEPTR)(lib))->library)

#define LC_NO_OPENLIB
#define LC_NO_CLOSELIB

#include <libcore/libheader.c>

#undef  SDEBUG
#undef  DEBUG
#define DEBUG 0
#include <aros/debug.h>

/* #define SysBase      (LC_SYSBASE_FIELD(lh)) */

#define AROSListBase lh

/* Global sysbase */
struct ExecBase *SysBase;

/* Predeclaration */
struct IClass *InitListClass(struct ListBase_intern *);

ULONG SAVEDS STDARGS LC_BUILDNAME(L_InitLib) (LC_LIBHEADERTYPEPTR lh)
{
    SysBase = LC_SYSBASE_FIELD(lh);
    
    if (!UtilityBase)
	UtilityBase = OpenLibrary("utility.library", 37);
    if (!UtilityBase)
	return(NULL);

    if (!IntuitionBase)
    	IntuitionBase = (IntuiBase *)OpenLibrary("intuition.library", 37);
    if (!IntuitionBase)
	return (NULL);

    /* ------------------------- */
    /* Create the class itself */

    if (!lh->classptr)
	lh->classptr = InitListClass(lh);
    if (!lh->classptr)
    	return (NULL);

    /* ------------------------- */

    return TRUE;
}


VOID SAVEDS STDARGS LC_BUILDNAME(L_ExpungeLib) (LC_LIBHEADERTYPEPTR lh)
{
    if (lh->classptr)
    {
	RemoveClass(lh->classptr);
	FreeClass(lh->classptr);
	lh->classptr = NULL;
    }

    CloseLibrary(UtilityBase);
    UtilityBase = NULL;

    CloseLibrary((struct Library *)IntuitionBase);
    IntuitionBase = NULL;

    return;

}
