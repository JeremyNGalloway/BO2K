/*  BOTOOL - Back Orifice 2000 File System and Registry Browser/Editor
    Copyright (C) 1999, L0pht Heavy Industries, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	The author of this program may be contacted at dildog@l0pht.com. */

#ifndef __INC_REGEDITFUNC_H
#define __INC_REGEDITFUNC_H

#include<windows.h>
#include"regedit.h"

#define WM_ENABLEITEMS (WM_USER+1)

#define IDC_STATUS 3000

typedef struct {
	HWND hParent;
} THREAD_ARGS;

BOOL RegeditDisconnect(REGEDIT_CONTEXT *recontext);
int RegeditConnect(REGEDIT_CONTEXT *recontext);
void SelectKey(REGEDIT_CONTEXT *recontext,HWND htv, HTREEITEM hti, HWND hlv);
void CollapseKey(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti);
void ExpandKey(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti);
BOOL RenameKey(REGEDIT_CONTEXT *recontext, HWND htv, TVITEM tvi);
BOOL RenameValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svOldValue, char *svNewValue);
BOOL DeleteKey(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti);
BOOL DeleteValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svValueName);
BOOL CreateNewKey(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svKeyName);
BOOL CreateNewValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, DWORD dwType, char *svName);
BOOL ModifyValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, DWORD dwType, char *svName, void *pNewData, DWORD dwNewDataLen);
void CopyKeyName(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti);

#endif