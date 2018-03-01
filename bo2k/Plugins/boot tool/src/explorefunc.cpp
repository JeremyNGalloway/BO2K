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

#include<windows.h>
#include<commctrl.h>
#include<plugins.h>
#include<bocomreg.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<config.h>
#include"comm_native.h"
#include"botool.h"
#include"explore.h"
#include<resource.h>
#include<afxres.h>
#include"strhandle.h"
#include"commnet.h"
#include<explorefunc.h>


// SortListItems: Sorts the file items in the list

int CALLBACK SortListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	EXPLORE_CONTEXT *excontext=(EXPLORE_CONTEXT *)lParamSort;
	EXFILEINFO *exfi1=(EXFILEINFO *)lParam1;
	EXFILEINFO *exfi2=(EXFILEINFO *)lParam2;
	
	int nSortDir=excontext->bSortDir[excontext->nLastSort]?-1:1;
	switch(excontext->nLastSort) {
	case 0:
		return lstrcmpi(exfi1->svName,exfi2->svName)*nSortDir;
	case 1:
		if(exfi1->dwSize<exfi2->dwSize) return -1*nSortDir;
		else if(exfi1->dwSize==exfi2->dwSize) return 0;
		else return 1*nSortDir;
	case 2:
		if(exfi1->nType<exfi2->nType) return -1*nSortDir;
		else if(exfi1->nType==exfi2->nType) {
			char *svExt1,*svExt2;
			if((svExt1=strrchr(exfi1->svName,'.'))==NULL) return -1*nSortDir;
			else if((svExt2=strrchr(exfi2->svName,'.'))==NULL) return 1*nSortDir;
			else return lstrcmpi(svExt1,svExt2);
		}
		else return 1*nSortDir;
		break;
	case 3:
		{
			FILETIME f1,f2;
			SystemTimeToFileTime(&(exfi1->stTime),&f1);
			SystemTimeToFileTime(&(exfi2->stTime),&f2);
			return CompareFileTime(&f1,&f2)*nSortDir;
		}
	case 4:
		if(exfi1->dwFlags<exfi2->dwFlags) return -1*nSortDir;
		else if(exfi1->dwFlags==exfi2->dwFlags) return 0;
		else return 1*nSortDir;
	}
	
	return 0;
}

// ExploreConnect: Prompts the user for connection information,
// connects an authenticated socket, and updates the dialog box

int ExploreConnect(EXPLORE_CONTEXT *excontext) 
{
	// Update dialog box
	HMENU menu=GetMenu(excontext->hDialog);
	HMENU connmenu=GetSubMenu(menu,3);

	// Create connection socket
	CAuthSocket *pSock=ConnectAuthSocket(InteractiveConnect,0,excontext->hDialog,
		"",
		GetCfgStr(g_szToolOptions,"Cmd Channel Net Module"),
		GetCfgStr(g_szToolOptions,"Cmd Channel Enc"),
		GetCfgStr(g_szToolOptions,"Cmd Channel Auth"));
	
	if(pSock==NULL || pSock==(CAuthSocket *)0xFFFFFFFF) {
		return (int)pSock;
	}

	excontext->pSock=pSock;
	
	// Modify title bar
	char svTitle[512];
	pSock->GetRemoteAddr(excontext->svAddress,256);
	wsprintf(svTitle,"File Browser - %.256s",excontext->svAddress);
	SetWindowText(excontext->hDialog,svTitle);
	
	// Modify location
	SetWindowText(GetDlgItem(excontext->hDialog,IDC_LOCATION),"");
	SendMessage(excontext->hDialog,WM_GOTOLOCATION,0,0);

	// Enable Disconnect Menu item
	EnableMenuItem(connmenu,IDM_DISCONNECT,MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(connmenu,IDM_CONNECT,MF_BYCOMMAND|MF_GRAYED);

	SendMessage(excontext->hDialog,WM_ENABLEITEMS,0,TRUE);

	SendMessage(excontext->hStatus,WM_SETTEXT,0,(LPARAM) "Connected");

	excontext->bCut=FALSE;
	excontext->svClipboard=NULL;

	// Set focus
	SetFocus(GetDlgItem(excontext->hDialog,IDC_LOCATION));
	
	return 1;
}

// ExploreDisconnect: Shuts down any socket, and updates the dialog box

BOOL ExploreDisconnect(EXPLORE_CONTEXT *excontext)
{
	if(excontext->svClipboard!=NULL) {
		excontext->bCut=FALSE;
		free(excontext->svClipboard);
		excontext->svClipboard=NULL;
	}

	if(excontext->pSock==NULL) return FALSE;

	excontext->pSock->Close();
	delete excontext->pSock;
	excontext->pSock=NULL;

	// Set location
	SetWindowText(GetDlgItem(excontext->hDialog,IDC_LOCATION),"");
	SendMessage(excontext->hDialog,WM_GOTOLOCATION,0,0);

	// Update dialog box
	HMENU menu=GetMenu(excontext->hDialog);
	HMENU connmenu=GetSubMenu(menu,3);

	// Enable Connect Menu item
	EnableMenuItem(connmenu,IDM_CONNECT,MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(connmenu,IDM_DISCONNECT,MF_BYCOMMAND|MF_GRAYED);

	// Modify title
	excontext->svAddress[0]='\0';
	SetWindowText(excontext->hDialog,"File Browser");

	SendMessage(excontext->hDialog,WM_ENABLEITEMS,0,FALSE);

	SendMessage(excontext->hStatus,WM_SETTEXT,0,(LPARAM) "Disconnected");
	
	return TRUE;
}

void UpdateFileList(EXPLORE_CONTEXT *excontext)
{
	HWND hwndlist=GetDlgItem(excontext->hDialog,IDC_FILELIST);
	CAuthSocket *pSock=excontext->pSock;
	
	// -------- Clear list -------------
	ListView_DeleteAllItems(hwndlist);

	// ------- Add all files to file list -------
	EXFILEINFO *pFile=excontext->pFiles;
	char svText[256];
	while(pFile!=NULL) {
		// Get file icon
		SHFILEINFO shfi;
		if(pFile->dwFlags & EXFI_D) {
			SHGetFileInfo("foobar",FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_SYSICONINDEX);
		} else {
			SHGetFileInfo(pFile->svName,pFile->dwFlags,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_SYSICONINDEX);
		}
		
		// Insert file
		LVITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iItem=0;
		lvi.iSubItem=0;
		lvi.pszText=pFile->svName;
		lvi.iImage=shfi.iIcon;
		lvi.lParam=(LPARAM)pFile;
		int nItem=ListView_InsertItem(hwndlist,&lvi);
		
		// Set file size
		if(pFile->nType!=0) {
			DWORD size=pFile->dwSize;
			if(size>(1024*1024)) {
				size/=(1024*1024);
				wsprintf(svText,"%uMB",size);
			} else if(size>1024) {
				size/=1024;
				wsprintf(svText,"%uKB",size);
			} else {
				wsprintf(svText,"%u",size);
			}
			lvi.mask=LVIF_TEXT;
			lvi.iItem=nItem;
			lvi.iSubItem=1;
			lvi.pszText=svText;
			ListView_SetItem(hwndlist,&lvi);
		}
		
		// Set file type
		lvi.mask=LVIF_TEXT;
		lvi.iItem=nItem;
		lvi.iSubItem=2;
		lvi.pszText=shfi.szTypeName;
		ListView_SetItem(hwndlist,&lvi);
		
		// Set file date/time
		wsprintf(svText,"%2.2d-%2.2d-%4.4d %2.2d:%2.2d",
			pFile->stTime.wMonth,
			pFile->stTime.wDay,
			pFile->stTime.wYear,
			pFile->stTime.wHour,
			pFile->stTime.wMinute);
		lvi.mask=LVIF_TEXT;
		lvi.iItem=nItem;
		lvi.iSubItem=3;
		lvi.pszText=svText;
		ListView_SetItem(hwndlist,&lvi);
		
		// Set file attributes
		svText[0]='\0';
		if(pFile->dwFlags & EXFI_A) lstrcat(svText,"A");
		if(pFile->dwFlags & EXFI_R) lstrcat(svText,"R");
		if(pFile->dwFlags & EXFI_S) lstrcat(svText,"S");
		if(pFile->dwFlags & EXFI_H) lstrcat(svText,"H");
		if(pFile->dwFlags & EXFI_D) lstrcat(svText,"D");
		if(pFile->dwFlags & EXFI_C) lstrcat(svText,"C");
		if(pFile->dwFlags & EXFI_T) lstrcat(svText,"T");
		
		lvi.mask=LVIF_TEXT;
		lvi.iItem=nItem;
		lvi.iSubItem=4;
		lvi.pszText=svText;
		ListView_SetItem(hwndlist,&lvi);
		
		// Move to next file
		pFile=pFile->pNext;
	}
	ListView_SortItems(hwndlist,SortListItems,(LPARAM)excontext);
}

// GoToDirectory

void GoToDirectory(EXPLORE_CONTEXT *excontext)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=excontext->pSock;
	HWND hwndlist=GetDlgItem(excontext->hDialog,IDC_FILELIST);
	
	char svText[256];
	// Get path to retrieve
	GetDlgItemText(excontext->hDialog,IDC_LOCATION,excontext->svLocation,259);
	int nLen=lstrlen(excontext->svLocation);
	if(nLen<=0) return;
	if(excontext->svLocation[nLen-1]!='\\') {
		lstrcat(excontext->svLocation,"\\");
		SetDlgItemText(excontext->hDialog,IDC_LOCATION,excontext->svLocation);
	}
	
	// Clear File List
	EXFILEINFO *pFile=excontext->pFiles,*pNext;
	while(pFile!=NULL) {
		pNext=pFile->pNext;
		free(pFile);
		pFile=pNext;
	}
	excontext->pFiles=NULL;
	
	// ---------- Request file list -----------
	
	// Issue command to server
	IssueAuthCommandRequest(pSock,BO_DIRECTORYLIST,0,0,excontext->svLocation,NULL);
	
	// Get "Contents of directory '...':"
	
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { ExploreDisconnect(excontext); return; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Unable",6)==0) {
		MessageBox(NULL,"The path specified can not be listed.","Directory Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		return;
	}
	if(strncmp(svArg2,"Contents",8)!=0) { 
		ExploreDisconnect(excontext); 
		pSock->Free(data);
		return; 
	}
	pSock->Free(data);
		
	// Get each file and add item
	int nCount=0;
	BOOL bDone=FALSE;
	while(!bDone) {
		// --------- Receive file info ----------
		Sleep(20);
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { ExploreDisconnect(excontext); return; }
		BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
		if(lstrlen(svArg2)>=8) {
			if(lstrcmp(svArg2+lstrlen(svArg2)-7,"files.\n")==0) {
				pSock->Free(data);
				bDone=TRUE;
				continue;
			}
		}
		
		// Skip . and .. directories
		wsprintf(svText,"%.255s",svArg2+48);
		BreakString(svText,"\n");
		if((lstrcmp(svText,"..")==0) || 
		   (lstrcmp(svText,".")==0)) {
			pSock->Free(data);
			continue;
		}

		// -------- Parse file info ----------
		pFile=(EXFILEINFO *)malloc(sizeof(EXFILEINFO));
		pFile->pNext=excontext->pFiles;
		excontext->pFiles=pFile;
		
		// Get msdos file name
		wsprintf(svText,"%.12s",svArg2);
		BreakString(svText," ");
		lstrcpyn(pFile->svShortName,svText,13);
		
		// Get file name
		wsprintf(svText,"%.255s",svArg2+48);
		BreakString(svText,"\n");
		lstrcpyn(pFile->svName,svText,260);
		
		// Get file size
		pFile->dwSize=atoi(svArg2+12);
		
		// Get file type
		if(*(svArg2+23)=='D') pFile->nType=0;
		else pFile->nType=1;
		
		// Get file date/time
		wsprintf(svText,"%.14s",svArg2+31);
		pFile->stTime.wMonth  = atoi(svText);
		pFile->stTime.wDay    = atoi(svText+3);
		pFile->stTime.wYear   = atoi(svText+6);
		pFile->stTime.wHour   = atoi(svText+11);
		pFile->stTime.wMinute = atoi(svText+13);
		pFile->stTime.wMilliseconds=0;
		pFile->stTime.wSecond=0;
		
		// Get file attributes
		pFile->dwFlags=0;
		wsprintf(svText,"%.7s",svArg2+23);
		int i;
		for(i=0;i<7;i++) {
			if(svText[i]=='A') pFile->dwFlags |= EXFI_A;
			if(svText[i]=='R') pFile->dwFlags |= EXFI_R;
			if(svText[i]=='S') pFile->dwFlags |= EXFI_S;
			if(svText[i]=='H') pFile->dwFlags |= EXFI_H;
			if(svText[i]=='D') pFile->dwFlags |= EXFI_D;
			if(svText[i]=='C') pFile->dwFlags |= EXFI_C;
			if(svText[i]=='T') pFile->dwFlags |= EXFI_T;
		}
		
		// Free info and move on
		pSock->Free(data);
		
		nCount++;
		wsprintf(svText,"Connected:  %d Files",nCount);
		SetWindowText(excontext->hStatus,svText);
		RECT r;
		GetWindowRect(excontext->hStatus,&r);
		MapWindowPoints(NULL,excontext->hDialog,(POINT *)&r,2);
		InvalidateRect(excontext->hDialog,&r,TRUE);
	}

	// Update file list view control
	UpdateFileList(excontext);
}					



// CreateNewFolder: Creates a new folder.
void CreateNewFolder(EXPLORE_CONTEXT *excontext) 
{
	int i,count,nRet,len;
	BYTE *data;
	char svName[260],svDir[260], *svFilePart;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	HWND hwndlist=GetDlgItem(excontext->hDialog,IDC_FILELIST);
	CAuthSocket *pSock=excontext->pSock;
		
	// Get folder name
	int nFolder=1;
	count=ListView_GetItemCount(hwndlist);
	do {
		if(nFolder==1) {
			wsprintf(svName,"New Folder");
		} else {
			wsprintf(svName,"New Folder %d",nFolder);
		}
		for(i=0;i<count;i++) {
			ListView_GetItemText(hwndlist,i,0,svDir,260);
			if(lstrcmpi(svDir,svName)==0) break;
		}
		nFolder++;
	} while(i!=count);

	lstrcpyn(svDir,excontext->svLocation,260);
	lstrcpyn(svDir+lstrlen(svDir),svName,260-lstrlen(svDir));
	GetFullPathName(svDir,260,svName,&svFilePart);
	
	// Issue create directory command
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,BO_DIRECTORYMAKE,comid,0,svName,NULL);
	
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { ExploreDisconnect(excontext); return; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,"A directory could not be created.","Directory Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		return;
	}
	if(strncmp(svArg2,"Directory",9)!=0) { 
		ExploreDisconnect(excontext); 
		pSock->Free(data);
		return; 
	}
	pSock->Free(data);
	
	// Add directory to list without reloading
	EXFILEINFO *pFile=(EXFILEINFO *)malloc(sizeof(EXFILEINFO));
	pFile->pNext=excontext->pFiles;
	excontext->pFiles=pFile;

	lstrcpyn(pFile->svName,svFilePart,260);
	pFile->dwSize=0;
	pFile->nType=0;
	GetLocalTime(&pFile->stTime);
	pFile->dwFlags=EXFI_D;

	// Update file list view control
	UpdateFileList(excontext);
	
	count=ListView_GetItemCount(hwndlist);
	for(i=0;i<count;i++) {
		ListView_GetItemText(hwndlist,i,0,svDir,260);
		if(lstrcmpi(svDir,svFilePart)==0) break;
	}
	if(i!=count) {
		ListView_EnsureVisible(hwndlist,i,FALSE);
		ListView_EditLabel(hwndlist,i);
	}
}

BOOL RenameItem(EXPLORE_CONTEXT *excontext, char *svOldName, char *svNewName, BOOL bCopy, BOOL bRelative)
{
	int nRet,len;
	BYTE *data;
	char svNewPath[260],svOldPath[260],svDir[260], *svFilePart;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	HWND hwndlist=GetDlgItem(excontext->hDialog,IDC_FILELIST);
	CAuthSocket *pSock=excontext->pSock;
		
	// Get item names
	if(bRelative) {
		lstrcpyn(svDir,excontext->svLocation,260);
		lstrcpyn(svDir+lstrlen(svDir),svOldName,260-lstrlen(svDir));
		GetFullPathName(svDir,260,svOldPath,&svFilePart);
	} else {
		GetFullPathName(svOldName,260,svOldPath,&svFilePart);
	}

	if(bRelative) {
		lstrcpyn(svDir,excontext->svLocation,260);
		lstrcpyn(svDir+lstrlen(svDir),svNewName,260-lstrlen(svDir));
		GetFullPathName(svDir,260,svNewPath,&svFilePart);
	} else {
		GetFullPathName(svNewName,260,svNewPath,&svFilePart);
	}
	
	// Issue create directory command
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,bCopy?BO_FILECOPY:BO_FILERENAME,comid,0,svOldPath,svNewPath);
	
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { ExploreDisconnect(excontext); return FALSE; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		if(bCopy) {
			MessageBox(NULL,"Item could not be copied","Copy error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		} else {
			MessageBox(NULL,"Item could not be renamed/moved.","Rename/move error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		}
		pSock->Free(data);
		return FALSE;
	}
	if(strncmp(svArg2,"File",4)!=0) { 
		ExploreDisconnect(excontext); 
		pSock->Free(data);
		return FALSE; 
	}
	pSock->Free(data);

	return TRUE;
}

void DeleteItems(EXPLORE_CONTEXT *excontext)
{
	int nRet,len;
	BYTE *data;
	char svPath[260],svDir[260], *svFilePart;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=excontext->pSock;
	int count,nItem;
	HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);

	count=ListView_GetItemCount(hlv);
	for(nItem=0;nItem<count;nItem++) {
		LVITEM lvi;
		lvi.iItem=nItem;
		lvi.iSubItem=0;
		lvi.mask=LVIF_TEXT|LVIF_STATE|LVIF_PARAM;
		lvi.stateMask=LVIS_SELECTED;
		lvi.pszText=svPath;
		lvi.cchTextMax=260;
		ListView_GetItem(hlv,&lvi);
		if(lvi.state & LVIS_SELECTED) {
			EXFILEINFO *exfi=(EXFILEINFO *)lvi.lParam;
			// Get item name
			lstrcpyn(svDir,excontext->svLocation,260);
			lstrcpyn(svDir+lstrlen(svDir),svPath,260-lstrlen(svDir));
			GetFullPathName(svDir,260,svPath,&svFilePart);			
			
			// Issue create directory command
			comid=GetTickCount();
			if(exfi->nType==0) {
				IssueAuthCommandRequest(excontext->pSock,BO_DIRECTORYDELETE,comid,0,svPath,NULL);
			} else {
				IssueAuthCommandRequest(excontext->pSock,BO_FILEDELETE,comid,0,svPath,NULL);
			}
			
			while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
			if(nRet==-1) { ExploreDisconnect(excontext); return; }
			BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
			if(strncmp(svArg2,"Could not",9)==0) {
				MessageBox(NULL,"Item could not be deleted.","Rename/move error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
				pSock->Free(data);
				return;
			}
			if((strncmp(svArg2,"File",4)!=0) &&
			   (strncmp(svArg2,"Directory",9)!=0)) { 
				ExploreDisconnect(excontext); 
				pSock->Free(data);
				return; 
			}
			pSock->Free(data);

			// Delete from listview
			ListView_DeleteItem(hlv,nItem);
			nItem--;
			count--;

			// Delete item from internal list
			EXFILEINFO *fi=excontext->pFiles;
			if(fi==exfi) {
				excontext->pFiles=exfi->pNext;
			} else while(fi!=NULL) {
				if(fi->pNext==exfi) {
					fi->pNext=exfi->pNext;
					break;
				}
				fi=fi->pNext;
			}
			free(exfi);	
		}
	}

}

void UploadFile(EXPLORE_CONTEXT *excontext,char *svFullName)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=excontext->pSock;

	// Get source and destination path
	char svSrcPath[260],*svFilePart;
	char svDstPath[260];
	GetFullPathName(svFullName,260,svSrcPath,&svFilePart);
	lstrcpyn(svDstPath,excontext->svLocation,260);
	lstrcpyn(svDstPath+lstrlen(svDstPath),svFilePart,260-lstrlen(svDstPath));

	// Open file to send
	HANDLE hFile=CreateFile(svSrcPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		char svMsg[512];
		wsprintf(svMsg,"Could not open %.256s",svSrcPath);
		MessageBox(excontext->hDialog,svMsg,"File open error",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
		return;
	}

	// Create upload listen socket on server side
	char svOpts[260];
	lstrcpyn(svOpts,excontext->svBindStr,260);
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Net Module"),260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Enc"),260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Auth"),260-lstrlen(svOpts));
requestrecvfile:;	
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,BO_RECEIVEFILE,comid,0,svOpts,svDstPath);
	
	while((nRet=pSock->Recv(&data,&len))==0) Sleep(20);
	if(nRet==-1) { ExploreDisconnect(excontext); CloseHandle(hFile); return; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		char svText[512];
		wsprintf(svText,"Message from server: %.256sDo you wish to try again?",svArg2);
		pSock->Free(data);
		if(MessageBox(NULL,svText,"Upload error",MB_YESNO|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST)==IDYES) {
			goto requestrecvfile;
		}
		CloseHandle(hFile);
		return;
	}
	if(strncmp(svArg2,"File",4)!=0) { 
		ExploreDisconnect(excontext); 
		pSock->Free(data);
		CloseHandle(hFile);
		return; 
	}
	
	// Get address of remote socket
	char *svAddr=svArg2+22;
	BreakString(svAddr,"\n");

	// Connect over and start sending file
	char *svNetMod,*svEnc,*svAuth;
	svNetMod=BreakString(svAddr,",");
	svEnc=BreakString(svNetMod,",");
	svAuth=BreakString(svEnc,",");
	
	CAuthSocket *pXferSock;
	if(excontext->svConnectStr[0]=='\0') {
		pXferSock=ConnectAuthSocket(NULL,0,excontext->hDialog,svAddr,svNetMod,svEnc,svAuth);
	} else {
		pXferSock=ConnectAuthSocket(NULL,0,excontext->hDialog,excontext->svConnectStr,svNetMod,svEnc,svAuth);
	}
	if(((int)pXferSock)<=0) {
		MessageBox(excontext->hDialog,"Could not connect to file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
		goto cancelupload;
	}
	
	DWORD dwTotalSize,dwSent;
	BY_HANDLE_FILE_INFORMATION bhfi;
	GetFileInformationByHandle(hFile,&bhfi);
	dwTotalSize=bhfi.nFileSizeLow;
	while((nRet=pXferSock->Send((BYTE *)&dwTotalSize,sizeof(DWORD)))<=0) Sleep(20);
	if(nRet<0) {
		MessageBox(excontext->hDialog,"Could not send to file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
		goto cancelupload;
	}

	HWND hwndXfer;
	hwndXfer=CreateXferDialog(g_hInstance, excontext->hDialog);
	XferDlg_SetSrcFile(hwndXfer,svSrcPath,NULL);
	XferDlg_SetDstFile(hwndXfer,svDstPath,svAddr);
 	XferDlg_SetTotalSize(hwndXfer,dwTotalSize);
	XferDlg_SetCount(hwndXfer,0);
	UpdateWindow(hwndXfer);
	
	for(dwSent=0;dwSent<dwTotalSize;dwSent+=4096) {
		if(PumpXferDialog(hwndXfer)==FALSE) {
			DestroyWindow(hwndXfer);
			pXferSock->Close();
			goto cancelupload;
		}
		
		BYTE pSendBuf[4096];
		DWORD dwBytes;
		ReadFile(hFile,pSendBuf,4096,&dwBytes,NULL);
		while((nRet=pXferSock->Send(pSendBuf,dwBytes))<=0) Sleep(20);
		if(nRet<0) {
			MessageBox(excontext->hDialog,"Could not send to file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
			DestroyWindow(hwndXfer);
			goto cancelupload;
		}
		
		XferDlg_SetCount(hwndXfer,dwSent);
		UpdateWindow(hwndXfer);
	}
	Sleep(1000);
	pXferSock->Close();
	DestroyWindow(hwndXfer);
	CloseHandle(hFile);
	return;

cancelupload:;
	// Cancel transfer
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,BO_CANCELTRANSFER,comid,0,NULL,svDstPath);
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	pSock->Free(data);
	CloseHandle(hFile);
	return;

}


void DownloadFile(EXPLORE_CONTEXT *excontext, char *svSrcPath, char *svDstPath)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=excontext->pSock;

	// Open file to send
	HANDLE hFile=CreateFile(svDstPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		char svMsg[512];
		wsprintf(svMsg,"Could not create %.256s",svDstPath);
		MessageBox(excontext->hDialog,svMsg,"File creation error",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
		return;
	}

	// Create download listen socket on server side
	char svOpts[260];
	lstrcpyn(svOpts,excontext->svBindStr,260);
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Net Module"),260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Enc"),260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),",",260-lstrlen(svOpts));
	lstrcpyn(svOpts+lstrlen(svOpts),GetCfgStr(g_szToolOptions,"File Xfer Auth"),260-lstrlen(svOpts));
requestemitfile:;	
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,BO_EMITFILE,comid,0,svOpts,svSrcPath);
	
	while((nRet=pSock->Recv(&data,&len))==0) Sleep(20);
	if(nRet==-1) { ExploreDisconnect(excontext); CloseHandle(hFile); return; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		char svText[512];
		wsprintf(svText,"Message from server: %.256sDo you wish to try again?",svArg2);
		pSock->Free(data);
		if(MessageBox(NULL,svText,"Download Error",MB_YESNO|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST)==IDYES) {
			goto requestemitfile;
		}
		CloseHandle(hFile);
		return;
	}
	if(strncmp(svArg2,"File",4)!=0) { 
		ExploreDisconnect(excontext); 
		pSock->Free(data);
		CloseHandle(hFile);
		return; 
	}
	
	// Get address of remote socket
	char *svAddr=svArg2+24;
	BreakString(svAddr,"\n");

	// Connect over and start sending file
	char *svNetMod,*svEnc,*svAuth;
	svNetMod=BreakString(svAddr,",");
	svEnc=BreakString(svNetMod,",");
	svAuth=BreakString(svEnc,",");
	
	CAuthSocket *pXferSock;
	if(excontext->svConnectStr[0]=='\0') {
		pXferSock=ConnectAuthSocket(NULL,0,excontext->hDialog,svAddr,svNetMod,svEnc,svAuth);
	} else {
		pXferSock=ConnectAuthSocket(NULL,0,excontext->hDialog,excontext->svConnectStr,svNetMod,svEnc,svAuth);
	}
	if(((int)pXferSock)<=0) {
		MessageBox(excontext->hDialog,"Could not connect to file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
		goto canceldownload;
	}
	
	DWORD dwTotalSize,dwRcvd,dwBytes;
	BYTE *pData;
	while((nRet=pXferSock->Recv(&pData,&len))==0) Sleep(20);
	if(nRet<0 || len!=sizeof(DWORD)) {
		MessageBox(excontext->hDialog,"Could not receive from file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
		goto canceldownload;
	}
	dwTotalSize=*(DWORD *)pData;
	pXferSock->Free(pData);

	HWND hwndXfer;
	hwndXfer=CreateXferDialog(g_hInstance, excontext->hDialog);
	XferDlg_SetSrcFile(hwndXfer,svSrcPath,svAddr);
	XferDlg_SetDstFile(hwndXfer,svDstPath,NULL);
 	XferDlg_SetTotalSize(hwndXfer,dwTotalSize);
	XferDlg_SetCount(hwndXfer,0);
	UpdateWindow(hwndXfer);
	
	dwRcvd=0;
	while(dwRcvd<dwTotalSize) {
		if(PumpXferDialog(hwndXfer)==FALSE) {
			DestroyWindow(hwndXfer);
			pXferSock->Close();
			goto canceldownload;
		}
		
		while((nRet=pXferSock->Recv(&pData,&len))==0) Sleep(20);
		if(nRet<0) {
			MessageBox(excontext->hDialog,"Could not receive from file xfer socket. Transfer cancelled.","Connection error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
			DestroyWindow(hwndXfer);
			goto canceldownload;
		}
		
		WriteFile(hFile,pData,len,&dwBytes,NULL);
		if(dwBytes!=(DWORD)len) {
			MessageBox(excontext->hDialog,"Error writing to file. Transfer cancelled.","File error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONINFORMATION);
			DestroyWindow(hwndXfer);
			goto canceldownload;
		}
		pXferSock->Free(pData);
		dwRcvd+=len;

		XferDlg_SetCount(hwndXfer,dwRcvd);
		UpdateWindow(hwndXfer);
	}
	Sleep(1000);
	pXferSock->Close();
	DestroyWindow(hwndXfer);
	CloseHandle(hFile);
	return;

canceldownload:;
	// Cancel transfer
	comid=GetTickCount();
	IssueAuthCommandRequest(excontext->pSock,BO_CANCELTRANSFER,comid,0,NULL,svSrcPath);
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	pSock->Free(data);
	CloseHandle(hFile);
	return;
}
