#ifndef WORKBENCH_WORKBENCH_H
#define WORKBENCH_WORKBENCH_H

/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: Workbench structures
    Lang: english
*/

#ifndef EXEC_LISTS_H
#   include <exec/lists.h>
#endif

#ifndef EXEC_NODES_H
#   include <exec/nodes.h>
#endif

#ifndef EXEC_TASKS_H
#   include <exec/tasks.h>
#endif

#ifndef EXEC_TYPES_H
#   include <exec/types.h>
#endif

#ifndef DOS_BPTR_H
#   include <dos/bptr.h>
#endif

#ifndef INTUITION_INTUITION_H
#   include <intuition/intuition.h>
#endif

/*** Workbench library name *************************************************/

#define WORKBENCHNAME           "workbench.library"

/*** Structures and associated definitions **********************************/

struct DrawerData {
    struct NewWindow dd_NewWindow;
    LONG             dd_CurrentX;
    LONG             dd_CurrentY;
    ULONG            dd_Flags;
    UWORD            dd_ViewModes;
};

#define DRAWERDATAFILESIZE (sizeof(struct DrawerData))

/* Definitions for dd_ViewModes */
#define DDVM_BYDEFAULT      0 /* Default (inherit parent's view mode) */
#define DDVM_BYICON         1 /* View as icons */
#define DDVM_BYNAME         2 /* View as text, sorted by name */
#define DDVM_BYDATE         3 /* View as text, sorted by date */
#define DDVM_BYSIZE         4 /* View as text, sorted by size */
#define DDVM_BYTYPE         5 /* View as text, sorted by type */

/* Definitions for dd_Flags */
#define DDFLAGS_SHOWDEFAULT 0 /* Default (show only icons) */
#define DDFLAGS_SHOWICONS   1 /* Show only icons */
#define DDFLAGS_SHOWALL     2 /* Show all files */

struct DiskObject
{
    UWORD               do_Magic;
    UWORD               do_Version;
    struct Gadget       do_Gadget;
    UBYTE               do_Type;        /* see below */
    char              * do_DefaultTool;
    char             ** do_ToolTypes;
    LONG                do_CurrentX;
    LONG                do_CurrentY;
    struct DrawerData * do_DrawerData;
    char              * do_ToolWindow;
    LONG                do_StackSize;
};

#define WBDISK    1
#define WBDRAWER  2
#define WBTOOL    3
#define WBPROJECT 4
#define WBGARBAGE 5
#define WBDEVICE  6
#define WBKICK    7
#define WBAPPICON 8

#define WB_DISKVERSION  1
#define WB_DISKREVISION 1
#define WB_DISKREVISIONMASK 0xFF

#define WB_DISKMAGIC 0xE310

struct FreeList {
    WORD        fl_NumFree;
    struct List fl_MemList;
};

/* Icons */
#define GFLG_GADGBACKFILL 0x0001
#define NO_ICON_POSITION  0x80000000

struct AppMessage {
    struct Message am_Message;
    UWORD          am_Type;     /* see below */
    ULONG          am_UserData;
    ULONG          am_ID;
    LONG           am_NumArgs;
    struct WBArg * am_ArgList;
    UWORD          am_Version;  /* see below */
    UWORD          am_Class;
    WORD           am_MouseX;
    WORD           am_MouseY;
    ULONG          am_Seconds;
    ULONG          am_Micros;
    ULONG          am_Reserved[8];
};

/* Definition for am_Version */
#define AM_VERSION         1

/* Definitions for am_Type */
#define AMTYPE_APPWINDOW   7
#define AMTYPE_APPICON     8
#define AMTYPE_APPMENUITEM 9

/* Definitions for am_Class */
#define AMCLASSICON_Open        0
#define AMCLASSICON_Copy        1
#define AMCLASSICON_Rename      2
#define AMCLASSICON_Information 3
#define AMCLASSICON_Snapshot    4
#define AMCLASSICON_UnSnapshot  5
#define AMCLASSICON_LeaveOut    6
#define AMCLASSICON_PutAway     7
#define AMCLASSICON_Delete      8
#define AMCLASSICON_FormatDisk  9
#define AMCLASSICON_EmptyTrash 10
#define AMCLASSICON_Selected   11
#define AMCLASSICON_Unselected 12

struct AppIconRenderMsg {
    struct RastPort   *arm_RastPort;
    struct DiskObject *arm_Icon;
    STRPTR             arm_Label;
    struct TagItem    *arm_Tags;
    WORD               arm_Left;
    WORD               arm_Top;
    WORD               arm_Width;
    WORD               arm_Height;
    ULONG              arm_State;
};

struct AppWindowDropZoneMsg {
    struct RastPort *adzm_RastPort;
    struct IBox      adzm_DropZoneBox;
    ULONG            adzm_ID;
    ULONG            adzm_UserData;
    LONG             adzm_Action;  /* see below */
};

/* Definitions for adzm_Action */
#define ADZMACTION_Enter (0)
#define ADZMACTION_Leave (1)

struct IconSelectMsg {
    ULONG           ism_Length;
    BPTR            ism_Drawer;
    STRPTR          ism_Name;
    UWORD           ism_Type;
    BOOL            ism_Selected;
    struct TagItem *ism_Tags;
    struct Window  *ism_DrawerWindow;
    struct Window  *ism_ParentWindow;
    WORD            ism_Left;
    WORD            ism_Top;
    WORD            ism_Width;
    WORD            ism_Height;
};

/* Hook return values */
#define ISMACTION_Unselect (0)
#define ISMACTION_Select   (1)
#define ISMACTION_Ignore   (2)
#define ISMACTION_Stop     (3)

/*** Private structures *****************************************************/

struct AppWindow;
struct AppWindowDropZone;
struct AppIcon;
struct AppMenuItem;

/*** Start of workbench.library tags ****************************************/

#define WBA_Dummy                          (TAG_USER+0xA000)

/*** Tags for use with AddAppIconA() ****************************************/

/* The different menu items the AppIcon responds to (BOOL) */
#define WBAPPICONA_SupportsOpen            (WBA_Dummy+1)
#define WBAPPICONA_SupportsCopy            (WBA_Dummy+2)
#define WBAPPICONA_SupportsRename          (WBA_Dummy+3)
#define WBAPPICONA_SupportsInformation     (WBA_Dummy+4)
#define WBAPPICONA_SupportsSnapshot        (WBA_Dummy+5)
#define WBAPPICONA_SupportsUnSnapshot      (WBA_Dummy+6)
#define WBAPPICONA_SupportsLeaveOut        (WBA_Dummy+7)
#define WBAPPICONA_SupportsPutAway         (WBA_Dummy+8)
#define WBAPPICONA_SupportsDelete          (WBA_Dummy+9)
#define WBAPPICONA_SupportsFormatDisk      (WBA_Dummy+10)
#define WBAPPICONA_SupportsEmptyTrash      (WBA_Dummy+11)

/* Propagate the AppIcons position back to original DiskObject (BOOL) */
#define WBAPPICONA_PropagatePosition       (WBA_Dummy+12)

/* Call this hook when rendering this AppIcon (struct Hook *) */
#define WBAPPICONA_RenderHook              (WBA_Dummy+13)

/* Notify the AppIcon when it's select state changes (BOOL) */
#define WBAPPICONA_NotifySelectState        (WBA_Dummy+14)

/*** Tags for use with AddAppMenuItemA() ************************************/

/* Command key string for this AppMenu (STRPTR) */
#define WBAPPMENUA_CommandKeyString         (WBA_Dummy+15)

/*** Tags for use with OpenWorkbenchObjectA() *******************************/

#define WBOPENA_ArgLock                     (WBA_Dummy+16)
#define WBOPENA_ArgName                     (WBA_Dummy+17)

/*** Tags for use with WorkbenchControlA() **********************************/

#define WBCTRLA_IsOpen                      (WBA_Dummy+18)
#define WBCTRLA_DuplicateSearchPath         (WBA_Dummy+19)
#define WBCTRLA_FreeSearchPath              (WBA_Dummy+20)
#define WBCTRLA_GetDefaultStackSize         (WBA_Dummy+21)
#define WBCTRLA_SetDefaultStackSize         (WBA_Dummy+22)
#define WBCTRLA_RedrawAppIcon               (WBA_Dummy+23)
#define WBCTRLA_GetProgramList              (WBA_Dummy+24)
#define WBCTRLA_FreeProgramList             (WBA_Dummy+25)
#define WBCTRLA_GetSelectedIconList         (WBA_Dummy+36)
#define WBCTRLA_FreeSelectedIconList        (WBA_Dummy+37)
#define WBCTRLA_GetOpenDrawerList           (WBA_Dummy+38)
#define WBCTRLA_FreeOpenDrawerList          (WBA_Dummy+39)
#define WBCTRLA_GetHiddenDeviceList         (WBA_Dummy+42)
#define WBCTRLA_FreeHiddenDeviceList        (WBA_Dummy+43)
#define WBCTRLA_AddHiddenDeviceName         (WBA_Dummy+44)
#define WBCTRLA_RemoveHiddenDeviceName      (WBA_Dummy+45)
#define WBCTRLA_GetTypeRestartTime          (WBA_Dummy+47)
#define WBCTRLA_SetTypeRestartTime          (WBA_Dummy+48)

/*** Tags for use with AddAppWindowDropZoneA() ******************************/

#define WBDZA_Left                          (WBA_Dummy+26)
#define WBDZA_RelRight                      (WBA_Dummy+27)
#define WBDZA_Top                           (WBA_Dummy+28)
#define WBDZA_RelBottom                     (WBA_Dummy+29)
#define WBDZA_Width                         (WBA_Dummy+30)
#define WBDZA_RelWidth                      (WBA_Dummy+31)
#define WBDZA_Height                        (WBA_Dummy+32)
#define WBDZA_RelHeight                     (WBA_Dummy+33)
#define WBDZA_Box                           (WBA_Dummy+34)
#define WBDZA_Hook                          (WBA_Dummy+35)

/*** Reserved tags **********************************************************/

#define WBA_Reserved1                       (WBA_Dummy+40)
#define WBA_Reserved2                       (WBA_Dummy+41)
#define WBA_Reserved3                       (WBA_Dummy+46)
#define WBA_Reserved4                       (WBA_Dummy+49)
#define WBA_Reserved5                       (WBA_Dummy+50)
#define WBA_Reserved6                       (WBA_Dummy+51)
#define WBA_Reserved7                       (WBA_Dummy+52)
#define WBA_Reserved8                       (WBA_Dummy+53)
#define WBA_Reserved9                       (WBA_Dummy+54)
#define WBA_Reserved10                      (WBA_Dummy+55)
#define WBA_Reserved11                      (WBA_Dummy+56)
#define WBA_Reserved12                      (WBA_Dummy+57)
#define WBA_Reserved13                      (WBA_Dummy+58)
#define WBA_Reserved14                      (WBA_Dummy+59)
#define WBA_Reserved15                      (WBA_Dummy+60)
#define WBA_Reserved16                      (WBA_Dummy+61)
#define WBA_Reserved17                      (WBA_Dummy+62)
#define WBA_Reserved18                      (WBA_Dummy+63)
#define WBA_Reserved19                      (WBA_Dummy+64)

/*** Last tag ***************************************************************/

#define WBA_LAST_TAG                        (WBA_Dummy+64)

#endif /* WORKBENCH_WORKBENCH_H */
