/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc: Basic support functions for layers.library.
    Lang: English.
*/

#include <aros/config.h>
#include <aros/asmcall.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <graphics/clip.h>
#include <graphics/regions.h>
#include <graphics/layers.h>
#include <graphics/gfx.h>
#include <utility/hooks.h>
#include <setjmp.h>

#include <proto/exec.h>
#include <proto/alib.h>
#include <proto/graphics.h>
#include <proto/layers.h>

#include "layers_intern.h"
#include "basicfuncs.h"

extern struct ExecBase * SysBase;

/*
 *  Sections:
 *
 *  + Blitter
 *  + Hook
 *  + Layer
 *  + LayerInfo
 *  + Rectangle
 *  + Resource Handling
 *  + Miscellaneous
 *
 */

/***************************************************************************/
/*                                 BLITTER                                 */
/***************************************************************************/

#define CR2NR_NOBITMAP 0
#define CR2NR_BITMAP   1

#if !(AROS_FLAVOUR & AROS_FLAVOUR_NATIVE)
/*
 * These functions cause the infamous "fixed or forbidden register was spilled"
 * bug/feature in m68k gcc, so these were written straight in asm. They can be
 * found in config/m68k-native/layers, for the m68k AROSfA target. Other targets,
 * that use stack passing, can use these versions.
 */

void BltRPtoCR(struct RastPort *    rp,
               struct ClipRect *    cr,
               ULONG                Mode)
{
    BltBitMap(rp->BitMap, cr->bounds.MinX, cr->bounds.MinY,
	      cr->BitMap, cr->bounds.MinX & 0xf, 0,
	      cr->bounds.MaxX - cr->bounds.MinX + 1,
	      cr->bounds.MaxY - cr->bounds.MinY + 1,
	      Mode, ~0, NULL);
}

void BltCRtoRP(struct RastPort *   rp,
               struct ClipRect *   cr,
               ULONG               Mode)
{
    BltBitMap(cr->BitMap, cr->bounds.MinX & 0xf, 0,
	      rp->BitMap, cr->bounds.MinX, cr->bounds.MinY,
	      cr->bounds.MaxX - cr->bounds.MinX + 1,
	      cr->bounds.MaxY - cr->bounds.MinY + 1,
	      Mode, ~0, NULL);
}

#endif /* if !native */

/***************************************************************************/
/*                                  HOOK                                   */
/***************************************************************************/

struct LayerHookMsg
{
    struct Layer *l;
/*  struct Rectangle rect; (replaced by the next line!) */
    WORD MinX, MinY, MaxX, MaxY;
    LONG OffsetX, OffsetY;
};

/***************************************************************************/
/*                                 LAYER                                   */
/***************************************************************************/

/* Set the priorities of the layer. The farther in the front it is the
   higher its priority will be.
 */
 
void SetLayerPriorities(struct Layer_Info * li)
{
  struct Layer * L = li -> top_layer;
  UWORD pri = 10000;
  while (NULL != L)
  {
    L -> priority = pri;
    pri--;
    L = L->back;
  }
}

struct Layer * internal_WhichLayer(struct Layer * l, WORD x, WORD y)
{
  while(l != NULL)
  {
    if(x >= l->bounds.MinX && x <= l->bounds.MaxX &&
       y >= l->bounds.MinY && y <= l->bounds.MaxY)
	     return l;
    l = l->back;
  }

  return NULL;
}


/***************************************************************************/
/*                               LAYERINFO                                 */
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*
 * Allocate LayerInfo_extra and initialize its resource list. Layers uses
 * this resource list to keep track of various memory allocations it makes
 * for the layers. See ResourceNode and ResData in layers_intern.h for the
 * node structure. See AddLayersResource for more information on the basic
 * operation.
 */
BOOL _AllocExtLayerInfo(struct Layer_Info * li)
{
    if(++li->fatten_count != 0)
	return TRUE;

    if(!(li->LayerInfo_extra = AllocMem(sizeof(struct LayerInfo_extra),MEMF_PUBLIC|MEMF_CLEAR)))
	return FALSE;

    NewList((struct List *)&((struct LayerInfo_extra *)li->LayerInfo_extra)->lie_ResourceList);

    return TRUE;
}

/*
 * Free LayerInfo_extra.
 */
void _FreeExtLayerInfo(struct Layer_Info * li)
{
    if(--li->fatten_count >= 0)
	return;

    if(li->LayerInfo_extra == NULL)
	return;

    FreeMem(li->LayerInfo_extra, sizeof(struct LayerInfo_extra));

    li->LayerInfo_extra = NULL;
}

/*
 * Initialize LayerInfo_extra and save the current environment.
 */
ULONG _InitLIExtra(struct Layer_Info * li,
                   struct LayersBase * LayersBase)
{
    struct LayerInfo_extra *lie = li->LayerInfo_extra;

    LockLayerInfo(li);

    /*
     * Initialize the ResourceList contained in the LayerInfo_extra.
     * This list is used to keep track of Layers' resource (memory/
     * bitmaps/etc.) allocations.
     */
    NewList((struct List *)&lie->lie_ResourceList);

    /*
     * Save the current environment, so we can drop back in case of
     * an error.
     */
    // return setjmp(lie->lie_JumpBuf);
    return 0;
}

void ExitLIExtra(struct Layer_Info * li,
                 struct LayersBase * LayersBase)
{
    struct LayerInfo_extra *lie = li->LayerInfo_extra;

    /* Free all resources associated with the layers. */
    FreeLayerResources(li, TRUE);

    UnlockLayerInfo(li);

    longjmp(lie->lie_JumpBuf, 1);
}

/*
 * Dynamically allocate LayerInfo_extra if it isn't already there.
 */
BOOL SafeAllocExtLI(struct Layer_Info * li,
                    struct LayersBase * LayersBase)
{
    LockLayerInfo(li);

    /* Check to see if we can ignore the rest of this call. :-) */
    if(li->Flags & NEWLAYERINFO_CALLED)
	return TRUE;

    if(_AllocExtLayerInfo(li))
	return TRUE;

    UnlockLayerInfo(li);

    return FALSE;
}

/*
 * Free LayerInfo_extra if it was dynamically allocated, and unlock the LI.
 */
void SafeFreeExtLI(struct Layer_Info * li,
                   struct LayersBase * LayersBase)
{
    if(!(li->Flags & NEWLAYERINFO_CALLED))
	_FreeExtLayerInfo(li);

    UnlockLayerInfo(li);
}

/***************************************************************************/
/*                                RECTANGLE                                */
/***************************************************************************/

/*
  Within the linked list of rectangles search for the rectangle that
  contains the given coordinates.
 */
struct ClipRect * internal_WhichClipRect(struct Layer * L, WORD x, WORD y)
{
  struct ClipRect * CR = L->ClipRect;
  while (NULL != CR)
  {
    if (x >= CR->bounds.MinX &&
        x <= CR->bounds.MaxX &&
        y >= CR->bounds.MinY &&
        y <= CR->bounds.MaxY)
      return CR;
    CR = CR->Next;
  }
  return NULL;
}

#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#define MIN(a,b)    ((a) < (b) ? (a) : (b))



/***************************************************************************/
/*                            RESOURCE HANDLING                            */
/***************************************************************************/

/*
 * Add a resource to the LayerInfo resource list, dynamically allocating
 * extra storage space if needed.
 */
BOOL AddLayersResource(struct Layer_Info * li,
                       void *              ptr,
                       ULONG               Size)
{
    struct ResourceNode *rn;
    struct ResData      *rd;

    if(!li)
	return TRUE;

    if(IsListEmpty(&((struct LayerInfo_extra *)li->LayerInfo_extra)->lie_ResourceList))
	if(!(rn = AddLayersResourceNode(li)))
	    return FALSE;

    /* Check to see if there are some entries left */
    if(--rn->rn_FreeCnt < 0)
    {
	/* If all entries are full, we have none left. Logic. :-) */
	rn->rn_FreeCnt = 0;

	/* So we add some more space for resources... */
	if(!(rn = AddLayersResourceNode(li)))
	    return FALSE;

	/* ...and decrement it for the following operations. */
	rn->rn_FreeCnt--;
    }

    rd = rn->rn_FirstFree++;

    rd->ptr  = ptr;
    rd->Size = Size;

    return TRUE;
}

/*
 * Add a new node to the LayerInfo resource list.
 */
struct ResourceNode * AddLayersResourceNode(struct Layer_Info * li)
{
    struct ResourceNode *rn;

    if(!(rn = (struct ResourceNode *)AllocMem(sizeof(struct ResourceNode), MEMF_ANY)))
	return NULL;

    /*
     * We keep 48 entries in this list. Could change depending on resource
     * allocation going on in Layers. For every n*48 allocations, a new node
     * must be allocated. This can (slightly) slow down operations if this
     * happens a lot.
     */
    rn->rn_FreeCnt   = 48;

    /* Point the cached pointer to the first free vector. */
    rn->rn_FirstFree = &rn->rn_Data[0];

    AddHead((struct List *)&((struct LayerInfo_extra *)li->LayerInfo_extra)->lie_ResourceList, &rn->rn_Link);

    return rn;
}

/*
 * Allocate memory for a ClipRect.
 */

struct ClipRect * _AllocClipRect(struct Layer_Info * LI)
{
  struct ClipRect * CR = NULL;

  /* I want to access the list of free ClipRects alone */
  ObtainSemaphore(&LI->Lock);

/*
  if (!IsListEmpty(&LI->FreeClipRects))
  {
    CR = (struct ClipRect *)RemHead((struct List *)&LI->FreeClipRects);  
    ReleaseSemaphore(&LI->Lock);
    CR->lobs = NULL;
    CR->BitMap = NULL;
    CR->bounds.MinX = 0;
    CR->bounds.MinY = 0;
    CR->bounds.MaxX = 0;
    CR->bounds.MaxY = 0;
    CR->Flags = 0;
    return CR;
  }
*/
  /* I am done, anybody else may get a ClipRect now */
  ReleaseSemaphore(&LI->Lock);

  CR =  AllocMem(sizeof(struct ClipRect), MEMF_PUBLIC|MEMF_CLEAR);

  return CR;  
}

/*
 * Return memory of a ClipRect for later use.
 */

void _FreeClipRect(struct ClipRect   * CR,
                   struct Layer_Info * LI)
{
  /* I want to access the list of ClipRects alone */
  ObtainSemaphore(&LI->Lock);
  
  /* Add the ClipRect to the list */
  AddHead((struct List *)&LI->FreeClipRects, (struct Node *)CR);

  /* I am done, anybody else may get a ClipRect now */
  ReleaseSemaphore(&LI->Lock);
}

/*
 * Allocate memory of a given size and enter it into the LayerInfo's
 * resource list.
 */
void * AllocLayerStruct(ULONG               Size,
                        ULONG               Flags,
                        struct Layer_Info * li,
                        struct LayersBase * LayersBase)
{
    void *mem;

    mem = AllocMem(Size, Flags);

    /* If there is no LayerInfo, this is just a straight AllocMem(). */
    if(li)
    {
	/* But if there is a LI, and there was an error, drop back to the
	   previous environment. */
	if(!mem)
	    ExitLIExtra(li, LayersBase);

	/* If not, enter the memory into the layers resource list. */
	if(!AddLayersResource(li, mem, Size))
	{
	    FreeMem(mem, Size);

	    /* Again, drop back in case of an error. */
	    ExitLIExtra(li, LayersBase);
	}
    }

    return mem;
}

/*
 * Traverse the ResourceList associated with the LayerInfo, and free all
 * allocated resources.
 */
void FreeLayerResources(struct Layer_Info * li,
                        BOOL                flag)
{
    struct ResourceNode *rn;
    struct ResData      *rd;
    ULONG                count;

    while( (rn = (struct ResourceNode *)
	RemHead((struct List *)&((struct LayerInfo_extra *)li->LayerInfo_extra)->lie_ResourceList)) )
    {
	if(flag)
	{
	    count = 48 - rn->rn_FreeCnt;

	    for(rd = &rn->rn_Data[0]; count-- != 0; rd++)
	    {
		if(rd->Size == RD_REGION)
		    DisposeRegion(rd->ptr);
		else if(rd->Size == RD_BITMAP)
		    FreeBitMap(rd->ptr);
		else
		    FreeMem(rd->ptr, rd->Size);
	    }

	    FreeMem(rn, sizeof(struct ResourceNode));
	}
    }

}

/***************************************************************************/
/*                              MISCELLANEOUS                              */
/***************************************************************************/



/*-----------------------------------END-----------------------------------*/
