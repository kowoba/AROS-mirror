/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function setjmp()
    Lang: english
*/

/******************************************************************************

    NAME
#include <setjmp.h>

	int setjmp (jmp_buf env);

    FUNCTION
	Save the current context so that you can return to it later.

    INPUTS
	env - The context/environment is saved here for later restoring

    RESULT
	0 if it returns from setjmp() and 1 when it returns from longjmp().

    NOTES

    EXAMPLE
	jmp_buf env;

	... some code ...

	if (!setjmp (env))
	{
	    ... this code is executed after setjmp() returns ...

	    // This is no good example on how to use this function
	    // You should not do that
	    if (error)
		longjmp (env, 5);

	    ... some code ...
	}
	else
	{
	    ... this code is executed if you call longjmp(env) ...
	}

    BUGS

    SEE ALSO
	longjmp()

    INTERNALS

    HISTORY

******************************************************************************/

	#include "machine.i"

	.text
	.balign 16
	.globl	AROS_CDEFNAME(setjmp)
	.type	AROS_CDEFNAME(setjmp),@function

	.set	FirstArg, 4 /* Skip Return-Adress */
	.set	env, FirstArg
	.set	retaddr, 0
	.set	reg_eax, 4

AROS_CDEFNAME(setjmp):
	/* Fetch the address of the env-structure off the stack.
	    The address is stored in %eax which is not preserved
	    because it's contents are overwritten anyway by the
	    return code */
	movl env(%esp),%eax

	/* Save stack pointer and all registers into env */
	movl %ebx,4(%eax) /* %ebx */
	movl %ecx,8(%eax) /* %ecx */
	movl %edx,12(%eax) /* %edx */
	movl %esi,16(%eax) /* %esi */
	movl %edi,20(%eax) /* %edi */
	movl %ebp,24(%eax) /* %ebp */
	movl %esp,28(%eax) /* %esp */

	pushl %ebx

	movl retaddr(%esp), %ebx /* Save return address */
	movl %ebx,0(%eax)

	popl %ebx

	xorl %eax,%eax /* Return 0 */
	ret
