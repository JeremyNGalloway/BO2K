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


#ifndef __INC_CMD_REGEDIT_H
#define __INC_CMD_REGEDIT_H

#include<windows.h>
#include<commctrl.h>

typedef struct {
	char svName[260];
	DWORD dwType;
	DWORD dwDataLen;
	void *pData;
} REGVALUE_INFO;

typedef struct {
	// Windows
	HWND hParent;
	HWND hDialog;
	HWND hStatus;
	HICON hLargeIcon;
	HICON hSmallIcon;
	HMENU hDlgMenu;
	HIMAGELIST hImageList;
	HFONT hFont;
	
	// Misc
	char svAddress[256];
	CAuthSocket *pSock;
	BOOL bWinNT;
	BOOL bEnter;

	// REGVALUE_INFO to modify in modification dialogs
	REGVALUE_INFO *pModRvi;
					
	// Keys
	HTREEITEM root;

	// New Key
	char svKeyName[260];
	BOOL bHex;
	// New Value
	void *pNewData;
	DWORD dwNewDataLen;

} REGEDIT_CONTEXT;

int CreateRegeditClient(HWND hParent);

#endif