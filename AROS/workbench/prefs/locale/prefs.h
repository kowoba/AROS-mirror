/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id: prefs.h 32341 2010-01-15 16:07:17Z mazze $

    Desc:
    Lang: English
*/

#ifndef _PREFS_H_
#define _PREFS_H_

#include <prefs/locale.h>
#include <dos/dos.h>
#include <intuition/classusr.h>

/*********************************************************************************************/

BOOL Prefs_Initialize(VOID);
VOID Prefs_Deinitialize(VOID);
BOOL Prefs_HandleArgs(STRPTR from, BOOL use, BOOL save);
BOOL Prefs_ImportFH(BPTR fh);
BOOL Prefs_ExportFH(BPTR fh);
BOOL Prefs_Default(VOID);

/*********************************************************************************************/

extern struct LocalePrefs localeprefs;

/*********************************************************************************************/

struct ListviewEntry
{
    struct Node node;
    char        name[30];
    char        realname[30];
    char        displayflag[100];
};

struct CountryEntry
{
    struct ListviewEntry lve;
    struct BitMap       *flagbm;
    WORD                 flagw;
    WORD                 flagh;
    Object              *pic;
    Object              *list_pic;

};

struct LanguageEntry
{
    struct ListviewEntry lve;
    BOOL                 preferred;
};

#define CHARACTER_SET_LEN 255

extern struct List          country_list, language_list, pref_language_list;
extern struct LocalePrefs   localeprefs;
extern char                 character_set[CHARACTER_SET_LEN];

#endif /* PREFS_H */
