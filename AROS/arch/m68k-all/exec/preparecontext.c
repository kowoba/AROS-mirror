/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: PrepareContext() - Prepare a task context for dispatch, m68k version
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <aros/m68k/cpucontext.h>
#include <proto/kernel.h>

#include "etask.h"
#include "exec_intern.h"
#include "exec_util.h"

#define _PUSH(sp, val) *--sp = (IPTR)val

AROS_LH4(BOOL, PrepareContext,
	 AROS_LHA(VOLATILE struct Task *, task,       A0),
	 AROS_LHA(APTR,                   entryPoint, A1),
	 AROS_LHA(APTR,                   fallBack,   A2),
	 AROS_LHA(struct TagItem *,       tagList,    A3),
	 struct ExecBase *, SysBase, 6, Exec)
{
    AROS_LIBFUNC_INIT

    IPTR args[8] = {0};
    WORD numargs = 0;
    IPTR *sp = task->tc_SPReg;
    struct ExceptionContext *ctx;

    if (!(task->tc_Flags & TF_ETASK) )
	return FALSE;
  
    ctx = KrnCreateContext();
    GetIntETask (task)->iet_Context = ctx;
    if (!ctx)
	return FALSE;

    while(tagList)
    {
    	switch(tagList->ti_Tag)
	{
	    case TAG_MORE:
	    	tagList = (struct TagItem *)tagList->ti_Data;
		continue;
		
	    case TAG_SKIP:
	    	tagList += tagList->ti_Data;
		break;
		
	    case TAG_DONE:
	    	tagList = NULL;
    	    	break;
		
#define HANDLEARG(x)					\
	    case TASKTAG_ARG ## x:			\
	    	args[x - 1] = (IPTR)tagList->ti_Data;	\
		if (x > numargs) numargs = x;		\
		break;

	    HANDLEARG(1)
	    HANDLEARG(2)
	    HANDLEARG(3)
	    HANDLEARG(4)
	    HANDLEARG(5)
	    HANDLEARG(6)
	    HANDLEARG(7)
	    HANDLEARG(8)
	}
	
	if (tagList) tagList++;
    }
    
    /*
	There is not much to do here, or at least that is how it
	appears. Most of the work is done in the kernel_cpu.h macros.
    */

    if (numargs)
    {
	/* On m68k C function gets all param on stack */
	while(numargs--)
	{
	    _PUSH(sp, args[numargs]);
	}
    }
    
    /* First we push the return address */
    _PUSH(sp, fallBack);
    
    /* Then set up the frame to be used by Dispatch() */
    ctx->pc   = (IPTR)entryPoint;
    ctx->a[7] = (IPTR)sp;
    ctx->sr   = 0x0000;

    /* We return the new stack pointer back to the caller. */
    task->tc_SPReg = sp;

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* PrepareContext() */
