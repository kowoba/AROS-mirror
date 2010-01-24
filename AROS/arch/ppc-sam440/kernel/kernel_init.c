#include <aros/kernel.h>
#include <aros/libcall.h>
#include <aros/symbolsets.h>
#include <inttypes.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include "memory.h"
#include <utility/tagitem.h>
#include <asm/amcc440.h>
#include <asm/io.h>
#include <strings.h>

#include <proto/exec.h>

#include "kernel_intern.h"
#include LC_LIBDEFS_FILE
#include "syscall.h"

/* forward declarations */
static void __attribute__((used)) kernel_cstart(struct TagItem *msg);

extern int exec_main(struct TagItem *msg, void *entry);

/* A very very very.....
 * ... very ugly code.
 * 
 * The AROS kernel gets executed at this place. The stack is unknown here, might be
 * set properly up, might be totally broken aswell and thus one cannot trust the contents
 * of %r1 register. Even worse, the kernel has been relocated most likely to some virtual 
 * address and the MMU mapping might not be ready now.
 * 
 * The strategy is to create one MMU entry first, mapping first 16MB of ram into last 16MB 
 * of address space in one turn and then making proper MMU map once the bss sections are cleared
 * and the startup routine in C is executed. This "trick" assumes two evil things:
 * - the kernel will be loaded (and will fit completely) within first 16MB of RAM, and
 * - the kernel will be mapped into top (last 16MB) of memory.
 * 
 * Yes, I'm evil ;) 
 */ 
 
asm(".section .aros.init,\"ax\"\n\t"
    ".globl start\n\t"
    ".type start,@function\n"
    "start:\n\t"
    "mr %r29,%r3\n\t"           /* Don't forget the message */
    "lis %r9,0xff00\n\t"        /* Virtual address 0xff000000 */
    "li %r10,0\n\t"             /* Physical address 0x00000000 */
    "ori %r9,%r9,0x0270\n\t"    /* 16MB page. Valid one */
    "li %r11,0x043f\n\t"        /* Write through cache. RWX enabled :) */
    "li %r0,0\n\t"              /* TLB entry number 0 */
    "tlbwe %r9,%r0,0\n\t"
    "tlbwe %r10,%r0,1\n\t"
    "tlbwe %r11,%r0,2\n\t"
    "isync\n\t"                         /* Invalidate shadow TLB's */
    "li %r9,0; mtspr 0x114,%r9; mtspr 0x115,%r9; mttbl %r9; mttbu %r9; mttbl %r9\n\t"
    "lis %r9,tmp_stack_end@ha\n\t"      /* Use temporary stack while clearing BSS */
    "lwz %r1,tmp_stack_end@l(%r9)\n\t"
    "bl __clear_bss\n\t"                /* Clear 'em ALL!!! */
    "lis %r11,target_address@ha\n\t"    /* Load the address of init code in C */
    "mr %r3,%r29\n\t"                   /* restore the message */
    "lwz %r11,target_address@l(%r11)\n\t"
    "lis %r9,stack_end@ha\n\t"          /* Use brand new stack to do evil things */
    "mtctr %r11\n\t"
    "lwz %r1,stack_end@l(%r9)\n\t"
    "bctr\n\t"                          /* And start the game... */
    ".string \"Native/CORE v3 (" __DATE__ ")\""
    "\n\t.text\n\t"
);

static void __attribute__((used)) __clear_bss(struct TagItem *msg) 
{
    struct KernelBSS *bss = (struct KernelBSS *) krnGetTagData(KRN_KernelBss, 0, msg);

    if (bss)
    {
        while (bss->addr && bss->len)
        {
            bzero(bss->addr, bss->len);
            bss++;
        }   
    }
}

static union {
    struct TagItem bootup_tags[64];
    uint32_t       tmp_stack  [128];
} tmp_struct                                   __attribute__((used, section(".data"), aligned(16)));
static const uint32_t *tmp_stack_end           __attribute__((used, section(".text"))) = &tmp_struct.tmp_stack[124];
static uint32_t        stack[STACK_SIZE]       __attribute__((used, aligned(16)));
static uint32_t        stack_super[STACK_SIZE] __attribute__((used, aligned(16)));
static const uint32_t *stack_end               __attribute__((used, section(".text"))) = &stack[STACK_SIZE-4];
static const void     *target_address          __attribute__((used, section(".text"))) = (void*)kernel_cstart;
static struct TagItem *BootMsg                 __attribute__((used));
static char            CmdLine[200]            __attribute__((used));

module_t *	modlist;
uint32_t	modlength;
void *priv_KernelBase;
uintptr_t	memlo;

static void __attribute__((used)) kernel_cstart(struct TagItem *msg)
{
    struct TagItem *tmp = tmp_struct.bootup_tags;

    /* Lowest usable kernel memory */
    memlo = 0xff000000;
    
    /* Disable interrupts and let FPU work */
    wrmsr((rdmsr() & ~(MSR_CE | MSR_EE | MSR_ME)) | MSR_FP);
        
    /* Enable FPU */
    wrspr(CCR0, rdspr(CCR0) & ~0x00100000);
    wrspr(CCR1, rdspr(CCR1) | (0x80000000 >> 24));

    /* First message after FPU is enabled, otherwise illegal instruction */
    D(bug("[KRN] Sam440 Kernel built on %s\n", __DATE__));

    /* Set supervisor stack */
    wrspr(SPRG0, (uint32_t)&stack_super[STACK_SIZE-4]);

    wrspr(SPRG4, 0);    /* Clear KernelBase */
    wrspr(SPRG5, 0);    /* Clear SysBase */

    D(bug("[KRN] Kernel resource pre-exec init\n"));
    D(bug("[KRN] MSR=%08x CRR0=%08x CRR1=%08x\n", rdmsr(), rdspr(CCR0), rdspr(CCR1)));
    D(bug("[KRN] USB config %08x\n", rddcr(SDR0_USB0)));
    
    D(bug("[KRN] msg @ %p\n", msg));
    D(bug("[KRN] Copying msg data\n"));
    while(msg->ti_Tag != TAG_DONE)
    {
        *tmp = *msg;

        if (tmp->ti_Tag == KRN_CmdLine)
        {
            strcpy(CmdLine, (char*) msg->ti_Data);
            tmp->ti_Data = (STACKIPTR) CmdLine;
            D(bug("[KRN] CmdLine %s\n", tmp->ti_Data));
        }
        else if (tmp->ti_Tag == KRN_BootLoader)
        {
        	tmp->ti_Data = (STACKIPTR) memlo;
        	memlo += (strlen(memlo) + 4) & ~3;
        	strcpy((char*)tmp->ti_Data, (const char*) msg->ti_Data);
        }
        else if (tmp->ti_Tag == KRN_DebugInfo)
        {
        	int i;
        	struct MinList *mlist = tmp->ti_Data;

        	D(bug("[KRN] DebugInfo at %08x\n", mlist));

        	module_t *mod = (module_t *)memlo;

        	ListLength(mlist, modlength);
        	modlist = mod;

        	memlo = &mod[modlength];

        	D(bug("[KRN] Bootstrap loaded debug info for %d modules\n", modlength));
        	/* Copy the module entries */
        	for (i=0; i < modlength; i++)
        	{
        		module_t *m = REMHEAD(mlist);
        		symbol_t *sym;

        		mod[i].m_lowest = m->m_lowest;
        		mod[i].m_highest = m->m_highest;
        		mod[i].m_str = NULL;
        		NEWLIST(&mod[i].m_symbols);
        		mod[i].m_name = (char *)memlo;
        		memlo += (strlen(m->m_name) + 4) & ~3;
        		strcpy(mod[i].m_name, m->m_name);

        		D(bug("[KRN] Module %s\n", m->m_name));

        		ForeachNode(&m->m_symbols, sym)
        		{
        			symbol_t *newsym = memlo;
        			memlo += sizeof(symbol_t);

        			newsym->s_name = memlo;
        			memlo += (strlen(sym->s_name)+4)&~3;
        			strcpy(newsym->s_name, sym->s_name);

        			newsym->s_lowest = sym->s_lowest;
        			newsym->s_highest = sym->s_highest;

        			ADDTAIL(&mod[i].m_symbols, newsym);
        		}
        	}

        	D(bug("[KRN] Debug info uses %d KB of memory\n", ((intptr_t)memlo - (intptr_t)mod) >> 10));

        }

        ++tmp;
        ++msg;
    }

    memlo = (memlo + 4095) & ~4095;

    BootMsg = tmp_struct.bootup_tags;
    D(bug("[KRN] BootMsg @ %p\n", BootMsg));

    /* Do a slightly more sophisticated MMU map */
    mmu_init(BootMsg);
    intr_init();
    
    /* 
     * Slow down the decrement interrupt a bit. Rough guess is that UBoot has left us with
     * 1kHz DEC counter.
     */
    wrspr(DECAR, 0xffffffff);

    /* Initialize exec.library */
    exec_main(BootMsg, NULL);

    goSuper();
    D(bug("[KRN] Uhm? Nothing to do?\n[KRN] STOPPED\n"));

    /* 
     * Do never ever try to return. This code would attempt to go back to the physical address
     * of asm trampoline, not the virtual one!
     */
    while(1) {
        wrmsr(rdmsr() | MSR_POW);
    }
}

AROS_LH0(void *, KrnCreateContext,
         struct KernelBase *, KernelBase, 10, Kernel)
{
    AROS_LIBFUNC_INIT

    context_t *ctx;

    uint32_t oldmsr = goSuper();

    ctx = Allocate(KernelBase->kb_SupervisorMem, sizeof(context_t));
    bzero(ctx, sizeof(context_t));

    wrmsr(oldmsr);

    if (!ctx)
        ctx = AllocMem(sizeof(context_t), MEMF_PUBLIC|MEMF_CLEAR);

    bug("[KRN] CreateContext()=%08x\n", ctx);

    return ctx;

    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, KrnDeleteContext,
                AROS_LHA(void *, context, A0),
         struct KernelBase *, KernelBase, 10, Kernel)
{
    AROS_LIBFUNC_INIT

    bug("[KRN] DeleteContext(%08x)\n", context);

    /* Was context in supervisor space? Deallocate it there :) */
    if ((intptr_t)context & 0xf0000000)
    {
        uint32_t oldmsr = goSuper();

        Deallocate(KernelBase->kb_SupervisorMem, context, sizeof(context_t));

        wrmsr(oldmsr);
    }
    else
        FreeMem(context, sizeof(context_t));

    /* Was this context owning a FPU? Make FPU totally free then */
    if (KernelBase->kb_FPUOwner == context)
        KernelBase->kb_FPUOwner = NULL;

    AROS_LIBFUNC_EXIT
}

AROS_LH0I(struct TagItem *, KrnGetBootInfo,
         struct KernelBase *, KernelBase, 10, Kernel)
{
    AROS_LIBFUNC_INIT

    return BootMsg;

    AROS_LIBFUNC_EXIT
}

struct MemHeader mh;

static int Kernel_Init(LIBBASETYPEPTR LIBBASE)
{
    int i;
    struct ExecBase *SysBase = getSysBase();
    
    uintptr_t krn_lowest  = krnGetTagData(KRN_KernelLowest,  0, BootMsg);
    uintptr_t krn_highest = krnGetTagData(KRN_KernelHighest, 0, BootMsg);
    
    D(bug("[KRN] Kernel resource post-exec init\n"));

    /* 4K granularity for data sections */
    krn_lowest &= 0xfffff000;
    /* 64K granularity for code sections */
    krn_highest = (krn_highest + 0xffff) & 0xffff0000;
    
    /* 
     * Set the KernelBase into SPRG4. At this stage the SPRG5 should be already set by
     * exec.library itself.
     */
    wrspr(SPRG4, LIBBASE);
    priv_KernelBase = LIBBASE;

    D(bug("[KRN] Allowing userspace to flush caches\n"));
    wrspr(MMUCR, rdspr(MMUCR) & ~0x000c0000);
    
    for (i=0; i < 16; i++)
        NEWLIST(&LIBBASE->kb_Exceptions[i]);

    for (i=0; i < 64; i++)
        NEWLIST(&LIBBASE->kb_Interrupts[i]);

    NEWLIST(&LIBBASE->kb_Modules);

    D(bug("[KRN] Preparing kernel private memory "));
    /* Prepare MemHeader structure to allocate from private low memory */
    mh.mh_Node.ln_Type    = NT_MEMORY;
    mh.mh_Node.ln_Pri     = 0;
    mh.mh_Node.ln_Name    = "Kernel Memory";
    mh.mh_Attributes      = MEMF_FAST | MEMF_KICK | MEMF_LOCAL;
    mh.mh_First           = (struct MemChunk *)memlo;
    mh.mh_Lower           = mh.mh_First;
    mh.mh_Upper           = (APTR) ((uintptr_t) 0xff000000 + krn_lowest - 1);

    mh.mh_Free            = (uintptr_t)mh.mh_Upper - (uintptr_t)mh.mh_Lower + 1;
    mh.mh_First->mc_Next  = NULL;
    mh.mh_First->mc_Bytes = mh.mh_Free;

    D(bug("[KRN] %08x - %08x, %d KB free\n", mh.mh_Lower, mh.mh_Upper, mh.mh_Free >> 10));

    LIBBASE->kb_SupervisorMem = &mh;

    /*
     * Add MemHeader about kernel memory to public MemList to avoid invalid
     * pointer debug messages for pointer that reference correctly into these
     * mem regions.
    */
    struct MemHeader *mh;
    mh                     = AllocMem(sizeof(struct MemHeader), MEMF_PUBLIC);
    mh->mh_Node.ln_Type    = NT_MEMORY;
    mh->mh_Node.ln_Pri     = -128;
    mh->mh_Node.ln_Name    = "Kernel Memory, Code + Data Sections";
    mh->mh_Attributes      = MEMF_FAST | MEMF_KICK | MEMF_LOCAL;
    mh->mh_First           = NULL;
    mh->mh_Free            = 0;
    mh->mh_Lower           = LIBBASE->kb_SupervisorMem;
    mh->mh_Upper           = (APTR) ((uintptr_t) 0xff000000 + krn_highest - 1);
    
    Enqueue(&SysBase->MemList, &mh->mh_Node);

    /* 
     * kernel.resource is ready to run. Enable external interrupts and leave 
     * supervisor mode.
     */
    wrmsr(rdmsr() | MSR_EE);
    D(bug("[KRN] Interrupts enabled\n"));

    goUser();
    D(bug("[KRN] Entered user mode \n"));

    Permit();

    return TRUE;
}

ADD2INITLIB(Kernel_Init, 0)
